/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "inference.h"

#define TENSOR_ARENA_SIZE (64 * 1024)
#define OUTPUT_BUFFER_SIZE 128

char output_buf[OUTPUT_BUFFER_SIZE];

// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

constexpr int kTensorArenaSize = TENSOR_ARENA_SIZE;
uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

void inference_setup() {
    // Set up logging. Google style is to avoid globals or statics because of
    // lifetime uncertainty, but since this has a trivial destructor it's okay.
    // NOLINTNEXTLINE(runtime-global-variables)
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    model = tflite::GetModel(g_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter,
                             "Model provided is schema version %d not equal "
                             "to supported version %d.",
                             model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    // This pulls in all the operation implementations we need.
    // NOLINTNEXTLINE(runtime-global-variables)
    static tflite::AllOpsResolver resolver;

    // Build an interpreter to run the model with.
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);

    if (input->type != kTfLiteInt8) {
        TF_LITE_REPORT_ERROR(error_reporter,
                             "Bad input tensor parameters in model");
        return;
    }
}

bool inference_exec(float data[], int len, ACTION* act, float* prob) {
    int i = 0;
    unsigned long start = 0;
    int8_t y_quantized = 0;
    float y[ACTION_MAX];

    //  Serial.println("[INF]Start");

    *act = ACTION_NONE;

    // Quantize the input from floating-point to integer
    for (i = 0; i < len; i++) {
        int8_t quantized_val =
            data[i] / input->params.scale + input->params.zero_point;
        input->data.int8[i] = quantized_val;
    }

    // Run inference, and report any error
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed");
        return false;
    }

    // Obtain the quantized output from model's output tensor
    y_quantized = output->data.int8[ACTION_NONE];
    y[ACTION_NONE] =
        (y_quantized - output->params.zero_point) * output->params.scale;

    y_quantized = output->data.int8[ACTION_GRAB];
    y[ACTION_GRAB] =
        (y_quantized - output->params.zero_point) * output->params.scale;

    y_quantized = output->data.int8[ACTION_HANDLE];
    y[ACTION_HANDLE] =
        (y_quantized - output->params.zero_point) * output->params.scale;

    y_quantized = output->data.int8[ACTION_LOCK];
    y[ACTION_LOCK] =
        (y_quantized - output->params.zero_point) * output->params.scale;

    y_quantized = output->data.int8[ACTION_DOOR];
    y[ACTION_DOOR] =
        (y_quantized - output->params.zero_point) * output->params.scale;

    sprintf(output_buf, "[INF]None:%f, Grab:%f, Handle:%f, Lock:%f, Door:%f",
            y[ACTION_NONE], y[ACTION_GRAB], y[ACTION_HANDLE], y[ACTION_LOCK],
            y[ACTION_DOOR]);
    Serial.println(output_buf);

    for (i = 1; i < ACTION_MAX; i++) {
        if (y[i] > y[*act]) {
            *act = (ACTION)i;
        }
    }
    *prob = y[*act];

    return true;
}
