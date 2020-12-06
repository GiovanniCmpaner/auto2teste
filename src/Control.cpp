
#include <Arduino.h>
#include <SPIFFS.h>

#include <cstdint>
#include <esp_log.h>

#include <TensorFlowLite_ESP32.h>

#include "tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "Control.hpp"
#include "Motors.hpp"
#include "Sensors.hpp"

namespace Control
{
    // Globals, used for compatibility with Arduino-style sketches.
    namespace
    {
        tflite::ErrorReporter *error_reporter = nullptr;
        const tflite::Model *model = nullptr;
        tflite::MicroInterpreter *interpreter = nullptr;
        TfLiteTensor *input = nullptr;
        TfLiteTensor *output = nullptr;
        int inference_count = 0;

        // Create an area of memory to use for input, output, and intermediate arrays.
        // Finding the minimum value for your model may require some trial and error.
        constexpr int kTensorArenaSize = 2 * 1024;
        uint8_t tensor_arena[kTensorArenaSize];
    } // namespace
    /*
    static auto load() -> void
    {
        // Set up logging. Google style is to avoid globals or statics because of
        // lifetime uncertainty, but since this has a trivial destructor it's okay.
        // NOLINTNEXTLINE(runtime-global-variables)
        static tflite::MicroErrorReporter micro_error_reporter{};
        error_reporter = &micro_error_reporter;

        // Map the model into a usable data structure. This doesn't involve any
        // copying or parsing, it's a very lightweight operation.
        tflite::FlatBufferModel
            model = tflite::GetModel(g_sine_data);
        if (model->version() != TFLITE_SCHEMA_VERSION)
        {
            error_reporter->Report(
                "Model provided is schema version %d not equal "
                "to supported version %d.",
                model->version(), TFLITE_SCHEMA_VERSION);
            return;
        }

        // This pulls in all the operation implementations we need.
        // NOLINTNEXTLINE(runtime-global-variables)
        static tflite::ops::micro::AllOpsResolver resolver;

        // Build an interpreter to run the model with.
        static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
        interpreter = &static_interpreter;

        // Allocate memory from the tensor_arena for the model's tensors.
        TfLiteStatus allocate_status = interpreter->AllocateTensors();
        if (allocate_status != kTfLiteOk)
        {
            error_reporter->Report("AllocateTensors() failed");
            return;
        }

        // Obtain pointers to the model's input and output tensors.
        input = interpreter->input(0);
        output = interpreter->output(0);
    }

    static auto inputs() -> std::vector<float>
    {
        auto inputs{std::vector<float>{}};
        {
            const auto distances{Sensors::distances()};
            for (const auto [angle, distance] : distances)
            {
                inputs.emplace_back(distance);
            }
        }
        {
            const auto colors{Sensors::colors()};
            for (auto color : colors)
            {
                inputs.emplace_back(color);
            }
        }
        return inputs;
    }

    static auto inference() -> void
    {
        // Calculate an x value to feed into the model. We compare the current
        // inference_count to the number of inferences per cycle to determine
        // our position within the range of possible x values the model was
        // trained on, and use this to calculate a value.
        float position = static_cast<float>(inference_count) /
                         static_cast<float>(kInferencesPerCycle);
        float x_val = position * kXrange;

        const auto inputs{Control::inputs()};
        memcpy(input->data.f, inputs.data(), inputs.size());

        // Run inference, and report any error
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk)
        {
            error_reporter->Report("Invoke failed on x_val: %f\n",
                                   static_cast<double>(x_val));
            return;
        }

        // Read the predicted y value from the model's output tensor
        {
            auto maxIndex{0};
            for (auto n{1}; n < 5; ++n)
            {
                if (output->data.f[maxIndex] < output->data.f[n])
                {
                    maxIndex = n;
                }
            }
            Motors::move(static_cast<Move>(maxIndex));
        }

        // Output the results. A custom HandleOutput function can be implemented
        // for each supported hardware target.
        HandleOutput(error_reporter, x_val, y_val);

        // Increment the inference_counter, and reset it if we have reached
        // the total number per cycle
        inference_count += 1;
        if (inference_count >= kInferencesPerCycle)
            inference_count = 0;
    }
    */
    auto init() -> void
    {
        log_d("begin");

        //Control::load();

        log_d("end");
    }

    auto process() -> void
    {
        //Control::inference();
    }

    auto mode(Mode modeValue) -> void
    {
    }
} // namespace Control