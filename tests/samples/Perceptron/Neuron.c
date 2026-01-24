#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define Dataset 1 // 0 - And | 1 - Or
#define LearningRate 0.1f

#define max(a, b) ((a) > (b) ? (a) : (b))

int Epoch = 1;

float Error;
float ErrorOccurred;

typedef struct {
    float *Weights;
    
    int InputAmount;
    float Bias;
    float Output;
} Neuron;

float Step(float Value) {
    return Value >= 0 ? 1.0f : 0.0f;
}

Neuron *CreateNeuron(int InputAmount) {
    Neuron *_Neuron = malloc(sizeof(Neuron));

    _Neuron -> Weights = malloc(InputAmount * sizeof(float));
    _Neuron -> InputAmount = InputAmount;
    _Neuron -> Bias = 0;
    
    for (int i = 0; i < InputAmount; i++) {
        _Neuron -> Weights[i] = 0;
    }

    return _Neuron;
}

float ForwardPass(Neuron *_Neuron, float *Inputs, float ExpectedOutput) {
    float Sum = 0.0f;

    for (int i = 0; i < _Neuron -> InputAmount; i++) {
        Sum += _Neuron -> Weights[i] * Inputs[i];
    }

    Sum += _Neuron -> Bias;
    _Neuron -> Output = Step(Sum);

    Error = ExpectedOutput - _Neuron -> Output;
    if (Error != 0)
        ErrorOccurred = 1;

    printf("Expected output: %.1f\n", ExpectedOutput);
    printf("Error: %.2f\n", Error);

    for (int i = 0; i < 2; i++) {
        _Neuron -> Weights[i] += LearningRate * Error * Inputs[i];
    }

    _Neuron -> Bias += LearningRate * Error;

    printf("Weighted sum (z): %.2f\n", Sum);

    return _Neuron -> Output;
}

int main() {
    float Inputs[4][2] = {
        {0, 0}, {0, 1}, {1, 0}, {1, 1}
    };

    float ExpectedOutputs[2][4] = {
        {0, 0, 0, 1}, {0, 1, 1, 1}
    };

    Neuron *_Neuron = CreateNeuron(2);

    int Activations = 0;

    printf("====================\n\n");

    do {
        ErrorOccurred = 0;
        Activations = 0;

        printf("Epoch: %i\n\n", Epoch);

        for (int i = 0; i <  4; i++) {
            float Output = ForwardPass(_Neuron, Inputs[i], ExpectedOutputs[Dataset][i]);
            if (_Neuron -> Output == 1.0f)
                Activations++;

            printf("Inputs: [%.0f, %.0f]\n", Inputs[i][0], Inputs[i][1]);
            printf("Bias: %.2f\n", _Neuron -> Bias);
            printf("Output: %.2f\n\n", Output);
        }

        printf("====================\n\n");

        Epoch++;
    } while (ErrorOccurred == 1);

    printf("Total Epoch's: %i\n", Epoch - 1);
    printf("Final Guess: %s\n", Activations == 1 ? "And" : "Or");

    free(_Neuron -> Weights);
    free(_Neuron);
}
