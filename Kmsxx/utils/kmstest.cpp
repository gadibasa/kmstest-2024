#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <drm/drm_mode.h>
#include <drm/drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

class DRMManager {
public:
    DRMManager(const std::string& device_path) {
        fd = open(device_path.c_str(), O_RDWR);
        if (fd < 0) {
            perror("Failed to open DRM device");
        } else {
            std::cout << "Using device: " << device_path << std::endl;
            std::cout << "File descriptor: " << fd << std::endl;
        }
    }

    ~DRMManager() {
        if (fd >= 0) {
            close(fd);
        }
    }

    void list_display_modes() const {
        if (fd < 0) return;

        drmModeRes *resources = drmModeGetResources(fd);
        if (!resources) {
            perror("Failed to get DRM resources");
            return;
        }

        std::cout << "Available connectors:\n";
        for (int i = 0; i < resources->count_connectors; i++) {
            drmModeConnector *conn = drmModeGetConnector(fd, resources->connectors[i]);
            if (conn) {
                std::cout << "Connector ID: " << conn->connector_id << " Type: " << conn->connector_type << " (" << conn->connector_type_id << ")\n";
                if (conn->connection == DRM_MODE_CONNECTED) {
                    std::cout << "  Status: Connected\n";
                    std::cout << "  Modes:\n";
                    for (int j = 0; j < conn->count_modes; j++) {
                        const drmModeModeInfo &mode = conn->modes[j];
                        std::cout << "    " << mode.name << " @ " << mode.vrefresh << " Hz\n";
                    }
                } else {
                    std::cout << "  Status: Disconnected\n";
                }
                drmModeFreeConnector(conn);
            }
        }
        drmModeFreeResources(resources);
    }

    void test_display_mode(uint32_t connector_id, drmModeModeInfo& mode) const {
        if (fd < 0) return;

        drmModeRes *resources = drmModeGetResources(fd);
        if (!resources) {
            perror("Failed to get DRM resources");
            return;
        }

        drmModeConnector *conn = drmModeGetConnector(fd, connector_id);
        if (!conn || conn->connection != DRM_MODE_CONNECTED) {
            std::cerr << "Connector " << connector_id << " is not connected\n";
            drmModeFreeResources(resources);
            return;
        }

        drmModeCrtc *crtc = drmModeGetCrtc(fd, resources->crtcs[0]);
        if (!crtc) {
            perror("Failed to get CRTC");
            drmModeFreeConnector(conn);
            drmModeFreeResources(resources);
            return;
        }

        int ret = drmModeSetCrtc(fd, crtc->crtc_id, 0, 0, 0, &connector_id, 1, &mode);
        if (ret) {
            perror("Failed to set mode");
        } else {
            std::cout << "Successfully set mode: " << mode.name << " @ " << mode.vrefresh << " Hz" << std::endl;
        }

        drmModeFreeCrtc(crtc);
        drmModeFreeConnector(conn);
        drmModeFreeResources(resources);
    }

    void fallback_display_modes() const {
        std::vector<std::string> fallback_modes = {
            "1920x1080@60",
            "2560x1440@60",
            "3840x2160@30"
        };

        drmModeRes *resources = drmModeGetResources(fd);
        if (!resources) {
            perror("Failed to get DRM resources");
            return;
        }

        bool edid_failed = true;
        for (int i = 0; i < resources->count_connectors; i++) {
            drmModeConnector *conn = drmModeGetConnector(fd, resources->connectors[i]);
            if (conn) {
                if (conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
                    edid_failed = false;
                }
                drmModeFreeConnector(conn);
            }
        }

        if (edid_failed) {
            std::cout << "EDID failed. Setting fallback modes...\n";
            for (int i = 0; i < resources->count_connectors; i++) {
                drmModeConnector *conn = drmModeGetConnector(fd, resources->connectors[i]);
                if (conn && conn->connection == DRM_MODE_CONNECTED) {
                    drmModeModeInfo mode = {};
                    strncpy(mode.name, "1920x1080", sizeof(mode.name) - 1);
                    mode.vrefresh = 60;
                    mode.hdisplay = 1920;
                    mode.vdisplay = 1080;
                    mode.vrefresh = 60;
                    mode.flags = DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC;
                    mode.type = DRM_MODE_TYPE_DRIVER;

                    drmModeCrtc *crtc = drmModeGetCrtc(fd, resources->crtcs[0]);
                    if (crtc) {
                        int ret = drmModeSetCrtc(fd, crtc->crtc_id, 0, 0, 0, &resources->connectors[i], 1, &mode);
                        if (ret) {
                            perror("Failed to set fallback mode");
                        } else {
                            std::cout << "Successfully set fallback mode: " << mode.name << " @ " << mode.vrefresh << " Hz" << std::endl;
                        }
                        drmModeFreeCrtc(crtc);
                    }
                    drmModeFreeConnector(conn);
                }
            }
        } else {
            std::cout << "EDID successful. No fallback modes needed.\n";
        }

        drmModeFreeResources(resources);
    }

private:
    int fd = -1;
};

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " --device=<device> <list|test|fallback> [options]\n";
        return -1;
    }

    std::string device_path = "/dev/dri/card0"; // Default device path
    std::string command;
    int i = 1;

    if (strncmp(argv[i], "--device=", 9) == 0) {
        device_path = std::string(argv[i] + 9);
        i++;
    }

    if (i < argc) {
        command = std::string(argv[i]);
        i++;
    }

    DRMManager drmManager(device_path);

    if (command == "list") {
        drmManager.list_display_modes();
    } else if (command == "test" && (i + 2) < argc) {
        uint32_t connector_id = std::stoul(argv[i]);
        drmModeModeInfo mode = {};
        std::string mode_name = argv[i + 1];
        unsigned int vrefresh = std::stoi(argv[i + 2]);

        // Simplified population of mode
        strncpy(mode.name, mode_name.c_str(), sizeof(mode.name) - 1);
        mode.vrefresh = vrefresh;

        drmManager.test_display_mode(connector_id, mode);
    } else if (command == "fallback") {
        drmManager.fallback_display_modes();
    } else {
        std::cerr << "Invalid command. Use <list|test|fallback>.\n";
    }

    return 0;
}
