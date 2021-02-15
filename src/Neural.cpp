
#include <Arduino.h>
#include <SPIFFS.h>

#include <cstdint>
#include <esp_log.h>
#include <numeric>

#include <TensorFlowLite_ESP32.h>
#include <tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h>
#include <tensorflow/lite/experimental/micro/micro_error_reporter.h>
#include <tensorflow/lite/experimental/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>

#include "Neural.hpp"

namespace Neural
{
    namespace
    {
        auto modelBuffer{std::vector<char>{}};
        auto errorReporter{static_cast<tflite::ErrorReporter *>(nullptr)};
        auto model{static_cast<const tflite::Model *>(nullptr)};
        auto interpreter{static_cast<tflite::MicroInterpreter *>(nullptr)};
        auto tensorArena{std::array<uint8_t, 2 * 1024>{}};

        auto inputTensor{static_cast<TfLiteTensor *>(nullptr)};
        auto outputTensor{static_cast<TfLiteTensor *>(nullptr)};

        auto load() -> void
        {
            if (not SPIFFS.exists("/model.tflite"))
            {
                log_d("model file not found");
                return;
            }

            auto file{SPIFFS.open("/model.tflite", FILE_READ)};
            if (not file)
            {
                log_e("model file error");
                return;
            }

            modelBuffer.resize(file.size());
            file.readBytes(modelBuffer.data(), modelBuffer.size());

            // Set up logging. Google style is to avoid globals or statics because of
            // lifetime uncertainty, but since this has a trivial destructor it's okay.
            // NOLINTNEXTLINE(runtime-global-variables)
            static auto micro_errorReporter{tflite::MicroErrorReporter{}};
            errorReporter = &micro_errorReporter;

            // Map the model into a usable data structure. This doesn't involve any
            // copying or parsing, it's a very lightweight operation.
            auto model{tflite::GetModel(modelBuffer.data())};
            if (model->version() != TFLITE_SCHEMA_VERSION)
            {
                log_e("version error");
                errorReporter->Report(
                    "Model provided is schema version %d not equal "
                    "to supported version %d.",
                    model->version(), TFLITE_SCHEMA_VERSION);

                return;
            }

            // This pulls in all the operation implementations we need.
            // NOLINTNEXTLINE(runtime-global-variables)
            static auto resolver{tflite::ops::micro::AllOpsResolver{}};

            // Build an interpreter to run the model with.
            static auto static_interpreter{tflite::MicroInterpreter{model, resolver, tensorArena.data(), tensorArena.size(), errorReporter}};
            interpreter = &static_interpreter;

            // Allocate memory from the tensor_arena for the model's tensors.
            auto allocate_status{interpreter->AllocateTensors()};
            if (allocate_status != kTfLiteOk)
            {
                log_e("allocate error");
                return;
            }

            // Obtain pointers to the model's inputTensor and outputTensor tensors.
            inputTensor = interpreter->input(0);
            outputTensor = interpreter->output(0);

            for (auto n{0}; n < interpreter->tensors_size(); ++n)
            {
                auto tensor{interpreter->tensor(n)};
                log_d("tensor[%d] = %s", n, tensor->name);
            }

            if (inputTensor->dims->size != 2 or inputTensor->dims->data[0] != 1 or inputTensor->dims->data[1] != 7)
            {
                errorReporter->Report("input size mismatch, expected [1,7]");
                return;
            }

            if (inputTensor->type != TfLiteType::kTfLiteFloat32)
            {
                errorReporter->Report("input type mismatch, expected FLOAT32");
                return;
            }

            if (outputTensor->dims->size != 2 or outputTensor->dims->data[0] != 1 or outputTensor->dims->data[1] != 5)
            {
                errorReporter->Report("output size mismatch, expected [1,5]");
                return;
            }

            if (outputTensor->type != TfLiteType::kTfLiteFloat32)
            {
                errorReporter->Report("output type mismatch, expected FLOAT32");
                return;
            }
        }

    } // namespace

    auto init() -> void
    {
        log_d("begin");

        Neural::load();

        log_d("end");
    }

    auto inference(const std::array<float, 7> &inputs) -> std::array<float, 5>
    {
        if (interpreter == nullptr)
        {
            log_e("interpreter error");
            return {};
        }

        std::memcpy(inputTensor->data.f, inputs.data(), inputs.size() * sizeof(float));

        // Run inference, and report any error
        const auto invoke_status{interpreter->Invoke()};
        if (invoke_status != kTfLiteOk)
        {
            log_e("invoke error");
            return {};
        }

        auto outputs{std::array<float, 5>{}};
        std::memcpy(outputs.data(), outputTensor->data.f, outputs.size() * sizeof(float));

        return outputs;
    }

} // namespace Neural