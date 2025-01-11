cd 
#include <iostream>
#include <cstring>
#include <vector>
#include <drm/drm_mode.h>
#include <drm/drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

void list_display_modes(int fd) {
    drmModeRes *resources = drmModeGetResources(fd);
    if (!resources) {
        perror("Failed to get DRM resources");
        return;
    }

    for (int i = 0; i < resources->count_connectors; i++) {
        drmModeConnector *conn = drmModeGetConnector(fd, resources->connectors[i]);
        if (conn && conn->connection == DRM_MODE_CONNECTED) {
            std::cout << "Connector " << conn->connector_id << " modes:\n";
            for (int j = 0; j < conn->count_modes; j++) {
                const drmModeModeInfo &mode = conn->modes[j];
                std::cout << "  " << mode.name << " @ " << mode.vrefresh << " Hz\n";
            }
        }
        drmModeFreeConnector(conn);
    }
    drmModeFreeResources(resources);
}

void test_display_mode(int fd, uint32_t connector_id, drmModeModeInfo mode) {
    drmModeRes *resources = drmModeGetResources(fd);
    if (!resources) {
        perror("Failed to get DRM resources");
        return;
    }

    drmModeCrtc *crtc = drmModeGetCrtc(fd, resources->crtcs[0]);
    if (!crtc) {
        perror("Failed to get CRTC");
        drmModeFreeResources(resources);
        return;
    }

    // Corrected API call with proper types
    int ret = drmModeSetCrtc(fd, crtc->crtc_id, 0, 0, 0, &connector_id, 1, &mode);
    if (ret) {
        perror("Failed to set mode");
    } else {
        std::cout << "Successfully set mode: " << mode.name << std::endl;
    }

    drmModeFreeCrtc(crtc);
    drmModeFreeResources(resources);
}


void fallback_display_modes() {
    std::vector<std::string> fallback_modes = {
        "1920x1080@60",
        "2560x1440@60",
        "3840x2160@30"
    };

    std::cout << "Fallback modes:\n";
    for (const auto &mode : fallback_modes) {
        std::cout << "  " << mode << "\n";
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <list|test|fallback> [options]\n";
        return -1;
    }

    int fd = drmOpen("card0", NULL);
    if (fd < 0) {
        perror("Failed to open DRM device");
        return -1;
    }

    if (strcmp(argv[1], "list") == 0) {
        list_display_modes(fd);
    } else if (strcmp(argv[1], "test") == 0 && argc == 4) {
        int connector_id = std::stoi(argv[2]);
        drmModeModeInfo mode = {}; // Populate based on user input or defaults.
        test_display_mode(fd, connector_id, mode);
    } else if (strcmp(argv[1], "fallback") == 0) {
        fallback_display_modes();
    } else {
        std::cerr << "Invalid command. Use <list|test|fallback>.\n";
    }

    drmClose(fd);
    return 0;
}
