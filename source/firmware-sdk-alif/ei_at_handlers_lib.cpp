#include "ei_device_info_lib.h"
// #include "ei_fusion.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
// #include "ei_run_impulse.h"
#include "at-server/ei_at_command_set.h"
#include "model-parameters/model_metadata.h"
#include "../ei_device_alif_e7.h"

#include <string>

using namespace std;

bool at_get_device_id(void)
{
    auto dev = EiDeviceInfo::get_device();
    ei_printf("%s\n", dev->get_device_id().c_str());

    return true;
}

bool at_set_device_id(const char **argv, const int argc)
{
    auto dev = EiDeviceInfo::get_device();
    if(argc < 1) {
        ei_printf("Missing argument!\n");
        return true;
    }

    dev->set_device_id(argv[0]);

    ei_printf("OK\n");

    return true;
}

bool at_get_upload_host(void)
{
    auto dev = EiDeviceInfo::get_device();
    ei_printf("%s\n", dev->get_upload_host().c_str());

    return true;
}

bool at_set_upload_host(const char **argv, const int argc)
{
    auto dev = EiDeviceInfo::get_device();
    if(argc < 1) {
        ei_printf("Missing argument!\n");
        return true;
    }

    dev->set_upload_host(argv[0]);

    ei_printf("OK\n");

    return true;
}

bool at_get_upload_settings(void)
{
    auto dev = EiDeviceInfo::get_device();
    ei_printf("Api Key:   %s\n", dev->get_upload_api_key().c_str());
    ei_printf("Host:      %s\n", dev->get_upload_host().c_str());
    ei_printf("Path:      %s\n", dev->get_upload_path().c_str());

    return true;
}

bool at_set_upload_settings(const char **argv, const int argc)
{
    auto dev = EiDeviceInfo::get_device();
    if(argc < 2) {
        ei_printf("Missing argument! Required: " AT_UPLOADSETTINGS_ARGS "\n");
        return true;
    }

    //TODO: can we set these values to ""?
    dev->set_upload_api_key(argv[0]);
    dev->set_upload_path(argv[1]);

    ei_printf("OK\n");

    return true;
}

bool at_get_mgmt_url(void)
{
    auto dev = EiDeviceInfo::get_device();
    ei_printf("%s\n", dev->get_management_url().c_str());

    return true;
}

bool at_set_mgmt_url(const char **argv, const int argc)
{
    auto dev = EiDeviceInfo::get_device();
    if(argc < 1) {
        ei_printf("Missing argument!\n");
        return true;
    }

    dev->set_management_url(argv[0]);

    ei_printf("OK\n");

    return true;
}

bool at_get_sample_settings(void)
{
    auto dev = EiDeviceInfo::get_device();
    ei_printf("Label:     %s\n", dev->get_sample_label().c_str());
    ei_printf("Interval:  %.2f ms.\n", dev->get_sample_interval_ms());
    ei_printf("Length:    %lu ms.\n", dev->get_sample_length_ms());
    ei_printf("HMAC key:  %s\n", dev->get_sample_hmac_key().c_str());

    return true;
}

bool at_set_sample_settings(const char **argv, const int argc)
{
    auto dev = EiDeviceInfo::get_device();
    if(argc < 3) {
        ei_printf("Missing argument! Required: " AT_SAMPLESETTINGS_ARGS "\n");
        return true;
    }

    dev->set_sample_label(argv[0]);

    //TODO: sanity check and/or exception handling
    string interval_ms_str(argv[1]);
    dev->set_sample_interval_ms(stof(interval_ms_str));

    //TODO: sanity check and/or exception handling
    string sample_length_str(argv[2]);
    dev->set_sample_length_ms(stoi(sample_length_str));

    if(argc >= 4) {
        dev->set_sample_hmac_key(argv[3]);
    }

    ei_printf("OK\n");

    return true;
}

bool at_clear_config(void)
{
    auto dev = EiDeviceInfo::get_device();
    dev->clear_config();
    //TODO wtf is this????  can someone explain device id to me?
    // dev->init_device_id();

    return true;
}

bool at_unlink_file(const char **argv, const int argc)
{
    ei_printf("\n");

    return true;
}

bool at_read_buffer(const char **argv, const int argc)
{
    auto dev = EiDeviceInfo::get_device();

    if(argc < 2) {
        ei_printf("Missing argument! Required: " AT_READBUFFER_ARGS "\n");
        return true;
    }
    bool success = true;

    size_t start = (size_t)atoi(argv[0]);
    size_t length = (size_t)atoi(argv[1]);
    bool use_max_baudrate = false;

    if (argc >= 3 && argv[2][0] == 'y') {
       use_max_baudrate = true;
    }

    if (use_max_baudrate) {
        ei_printf("OK\r\n");
        ei_sleep(100);
        dev->set_max_data_output_baudrate();
    }

    success = dev->read_encode_send_sample_buffer(start, length);

    if (use_max_baudrate) {
        ei_printf("\r\nOK\r\n");
        ei_sleep(100);
        dev->set_default_data_output_baudrate();
    }

    if (!success) {
        ei_printf("Failed to read from buffer\n");
    }
    else {
        ei_printf("\n");
    }

    return true;
}

bool at_sample_start(const char **argv, const int argc)
{
    auto dev = EiDeviceInfo::get_device();

    if(argc < 1) {
        ei_printf("Missing sensor name!\n");
        return true;
    }

    const ei_device_sensor_t *sensor_list;
    size_t sensor_list_size;

    dev->get_sensor_list((const ei_device_sensor_t **)&sensor_list, &sensor_list_size);

    for (size_t ix = 0; ix < sensor_list_size; ix++) {
        if (strcmp(sensor_list[ix].name, argv[0]) == 0) {
            if (!sensor_list[ix].start_sampling_cb()) {
                ei_printf("ERR: Failed to start sampling\n");
            }
            return true;
        }
    }
    //TODO
    // if (ei_connect_fusion_list(argv[0], SENSOR_FORMAT)) {
    //     if (!ei_fusion_setup_data_sampling(dev)) {
    //         ei_printf("ERR: Failed to start sensor fusion sampling\n");
    //     }
    // }
    // else {
    //     ei_printf("ERR: Failed to find sensor '%s' in the sensor list\n", argv[0]);
    // }


    return true;
}

// TODO, and to add abstraction layer?
// bool at_run_impulse(void)
// {
//     ei_start_impulse(false, false);

//     return false;
// }

// bool at_run_impulse_debug(void)
// {
//     ei_start_impulse(false, true);

//     return false;
// }

// bool at_run_impulse_cont(void)
// {
//     ei_start_impulse(true, true);

//     return false;
// }

// bool at_stop_impulse(void)
// {
//     ei_stop_impulse();

//     return true;
// }


bool at_get_snapshot(void) {

    const ei_device_snapshot_resolutions_t *list;
    size_t list_size;
    const char *color_depth;

    int r =  EiDeviceInfo::get_device()->get_snapshot_list((const ei_device_snapshot_resolutions_t **)&list, &list_size, &color_depth);
    if (r) { /* apparently false is OK here?! */
        ei_printf("Has snapshot:    0\n");
        return true;
    }

    ei_printf("Has snapshot:         1\n");
    ei_printf("Supports stream:      1\n");
    ei_printf("Color depth:          %s\n", color_depth);
    ei_printf("Resolutions:          [ ");
    for (size_t ix = 0; ix < list_size; ix++) {
        ei_printf("%ux%u", list[ix].width, list[ix].height);
        if (ix != list_size - 1) {
            ei_printf(", ");
        }
        else {
            ei_printf(" ");
        }
    }
    ei_printf("]\n");

    return true;
}


bool at_get_config(void)
{
    auto dev = EiDeviceInfo::get_device();

    ei_printf("===== Device info =====\n");
    ei_printf("ID:         %s\n", dev->get_device_id().c_str());
    ei_printf("Type:       %s\n", dev->get_device_type().c_str());
    ei_printf("AT Version: " AT_COMMAND_VERSION "\n");
    ei_printf("\n");

    ei_printf("===== Sensors ======\n");
    const ei_device_sensor_t *sensor_list;
    size_t sensor_list_size;
    dev->get_sensor_list((const ei_device_sensor_t **)&sensor_list, &sensor_list_size);
    for (size_t ix = 0; ix < sensor_list_size; ix++) {
        ei_printf("Name: %s, Max sample length: %hus, Frequencies: [", sensor_list[ix].name, sensor_list[ix].max_sample_length_s);
        for (size_t fx = 0; fx < EI_MAX_FREQUENCIES; fx++) {
            if (sensor_list[ix].frequencies[fx] != 0.0f) {
                if (fx != 0) {
                    ei_printf(", ");
                }
                ei_printf("%.2fHz", sensor_list[ix].frequencies[fx]);
            }
        }
        ei_printf("]\n");
    }
    // ei_built_sensor_fusion_list(dev);
    ei_printf("\n");
    ei_printf("===== WIFI =====\n");
    dev->print_wifi_config();
    ei_printf("\n");
    ei_printf("===== Sampling parameters =====\n");
    ei_printf("Label:     %s\n", dev->get_sample_label().c_str());
    ei_printf("Interval:  %.2f ms.\n", dev->get_sample_interval_ms());
    ei_printf("Length:    %lu ms.\n", dev->get_sample_length_ms());
    ei_printf("HMAC key:  %s\n", dev->get_sample_hmac_key().c_str());
    ei_printf("\n");
    ei_printf("===== Upload settings =====\n");
    ei_printf("Api Key:   %s\n", dev->get_upload_api_key().c_str());
    ei_printf("Host:      %s\n", dev->get_upload_host().c_str());
    ei_printf("Path:      %s\n", dev->get_upload_path().c_str());
    ei_printf("\n");
    ei_printf("===== Remote management =====\n");
    ei_printf("URL:        %s\n", dev->get_management_url().c_str());
    ei_printf("Connected:  0\n");
    ei_printf("Last error: \n");
    ei_printf("\n");
    ei_printf("===== Snapshot ======\n");
    at_get_snapshot();
    ei_printf("===== Inference ======\n");
    ei_printf("Sensor:           %d\r\n", EI_CLASSIFIER_SENSOR);
#if EI_CLASSIFIER_OBJECT_DETECTION_CONSTRAINED == 1
    const char *model_type = "constrained_object_detection";
#elif EI_CLASSIFIER_OBJECT_DETECTION
    const char *model_type = "object_detection";
#else
    const char *model_type = "classification";
#endif
    ei_printf("Model type:       %s\r\n", model_type);

    return true;
}