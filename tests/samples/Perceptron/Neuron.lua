Dataset = 1 -- 0 : And | 1 : Or
LearningRate = 0.1

Epoch = 1
Activations = 0

Error = 0
ErrorOccurred = false

function Max(a, b)
    if a > b then
        return a
    else
        return b
    end
end

function Step(Value)
    if Value >= 0 then
        return 1.0
    else
        return 0.0
    end
end

function CreateNeuron(InputAmount)
    local Neuron = {}

    Neuron.Weights = {}

    for i = 1, InputAmount do
        Neuron.Weights[i] = 0.0
    end

    Neuron.Bias = 0.0
    Neuron.Output = 0.0

    return Neuron
end

function ForwardPass(Neuron, Inputs, ExpectedOutput)
    local Sum = 0.0

    for i = 1, #Inputs do
        Sum = Sum + Inputs[i] * Neuron.Weights[i]
    end

    Sum = Sum + Neuron.Bias
    Neuron.Output = Step(Sum)

    Error = ExpectedOutput - Neuron.Output
    if Error ~= 0 then
        ErrorOccurred = true
    end

    print("Expected: " .. ExpectedOutput)
    print("Error: " .. Error)

    for i = 1, #Inputs do
        Neuron.Weights[i] = Neuron.Weights[i] + LearningRate * Error * Inputs[i]
    end

    Neuron.Bias = Neuron.Bias + LearningRate * Error

    print("Weighted sum (z): " .. Sum)

    return Neuron.Output
end

local Inputs = {
    {0.0, 0.0},
    {0.0, 1.0},
    {1.0, 0.0},
    {1.0, 1.0}
}

local ExpectedOutputs = {
    {0.0, 0.0, 0.0, 1.0},
    {0.0, 1.0, 1.0, 1.0}
}

local Neuron = CreateNeuron(2)

print("====================\n")

while true do
    print("Epoch: " .. Epoch)

    ErrorOccurred = false

    for i = 1, #Inputs do
        print("")

        local Output = ForwardPass(Neuron, Inputs[i], ExpectedOutputs[Dataset + 1][i])

        print("Input: [" .. Inputs[i][1] .. ", " .. Inputs[i][2] .. "]")
        print("Bias: " .. Neuron.Bias)
        print("Output: " .. Output)
    end

    print("\n====================\n")

    if not ErrorOccurred then
        break
    end
    
    Epoch = Epoch + 1
end

print("Total Epochs: " .. Epoch)

if Activations == 1 then
    print("Final Guess: AND")
else
    print("Final Guess: OR")
end
