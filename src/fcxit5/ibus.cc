#include "ibus.h"
#include <sys/stat.h>
#include "../../third-party/glfw/include/GLFW/glfw3.h"
#include <iostream>



static const char*
get_ibus_address_file_name(void) {
    const char *addr;
    static char ans[PATH_MAX];
    static char display[64] = {0};
    addr = getenv("IBUS_ADDRESS");
    int offset = 0;
    if (addr && addr[0]) {
        memcpy(ans, addr, std::min(strlen(addr), sizeof(ans)));
        return ans;
    }
    const char* disp_num = NULL;
    const char *host = "unix";
    // See https://github.com/ibus/ibus/commit/8ce25208c3f4adfd290a032c6aa739d2b7580eb1 for why we need this dance.
    const char *de = getenv("WAYLAND_DISPLAY");
    if (de) {
        disp_num = de;
    } else {
        const char *de = getenv("DISPLAY");
        if (!de || !de[0]) de = ":0.0";
        strncpy(display, de, sizeof(display) - 1);
        char *dnum = strrchr(display, ':');
        if (!dnum) {
            // _glfwInputError(GLFW_PLATFORM_ERROR, "Could not get IBUS address file name as DISPLAY env var has no colon");
            return NULL;
        }
        char *screen_num = strrchr(display, '.');
        *dnum = 0;
        dnum++;
        if (screen_num) *screen_num = 0;
        if (*display) host = display;
        disp_num = dnum;
    }

    memset(ans, 0, sizeof(ans));
    const char *conf_env = getenv("XDG_CONFIG_HOME");
    if (conf_env && conf_env[0]) {
        offset = snprintf(ans, sizeof(ans), "%s", conf_env);
    } else {
        conf_env = getenv("HOME");
        if (!conf_env || !conf_env[0]) {
            // _glfwInputError(GLFW_PLATFORM_ERROR, "Could not get IBUS address file name as no HOME env var is set");
            return NULL;
        }
        offset = snprintf(ans, sizeof(ans), "%s/.config", conf_env);
    }
    char *key = dbus_get_local_machine_id();
    snprintf(ans + offset, sizeof(ans) - offset, "/ibus/bus/%s-%s-%s", key, host, disp_num);
    dbus_free(key);
    return ans;
}

static DBusHandlerResult
message_handler(DBusConnection *conn , DBusMessage *msg, void *user_data) {
	LeditIBus* bus = reinterpret_cast<LeditIBus*>(user_data);
	    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

}
void LeditIBus::connect() {
	if(connected)
		return;
	ibusData.inited = true;
    ibusData.name_owner_changed = false;
    const char *client_name = "GLFW_Application";
    const char *address_file_name = get_ibus_address_file_name();
  	ibusData.ok = false;
  	ibusData.address_file_name = std::string(address_file_name);
    if(!readAddress())
        return;
	ibusData.conn = dBus.connectTo(ibusData.address.c_str(), "Failed to connect to the IBUS daemon, with error", "ibus", true);
	if(!ibusData.conn){
		std::cout << "no ibus connection\n";
		return;
	}
	state = 1;
	ibusData.input_ctx_path= "";
 if (!ibusData.conn->callMethodWithReply(IBUS_SERVICE, IBUS_PATH, IBUS_INTERFACE, "CreateInputContext", DBUS_TIMEOUT_USE_DEFAULT, [&](DBusMessage *msg, const char* err, void* data) {
 	onInputContextCreated(msg);
	}, &ibusData, DBUS_TYPE_STRING, &client_name, DBUS_TYPE_INVALID)) {
        return;
    }	
    std::cout << "lol\n";

}
void LeditIBus::onInputContextCreated(DBusMessage *msg) {
	const char *path = NULL;
    if (!ibusData.conn->getArgs(msg, "Failed to get IBUS context path from reply", DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID)) return;
    ibusData.input_ctx_path = std::string(path);
    if(!ibusData.input_ctx_path.length())
    	return;
    dbus_bus_add_match(ibusData.conn->conn(), "type='signal',interface='org.freedesktop.DBus', member='NameOwnerChanged'", NULL);
    dbus_connection_add_filter(ibusData.conn->conn(), ibus_on_owner_change, &ibusData, free);
    dbus_bus_add_match(ibusData.conn->conn(), "type='signal',interface='org.freedesktop.IBus.InputContext'", NULL);
    DBusObjectPathVTable ibus_vtable = {.message_function = message_handler};
    dbus_connection_try_register_object_path(ibusData.conn->conn(), ibusData.input_ctx_path.c_str(), &ibus_vtable, this, NULL);
    Capabilities caps = (Capabilities)(Capabilities::IBUS_CAP_FOCUS | Capabilities::IBUS_CAP_PREEDIT_TEXT);
    if (!ibusData.conn->callMethodNoReply(IBUS_SERVICE, ibusData.input_ctx_path.c_str(), IBUS_INPUT_INTERFACE, "SetCapabilities", DBUS_TYPE_UINT32, &caps, DBUS_TYPE_INVALID)) return;
    ibusData.ok = true;
        connected = true;
    // glfw_ibus_set_focused(ibus, _glfwFocusedWindow() != NULL);
    // glfw_ibus_set_cursor_geometry(ibus, 0, 0, 0, 0);
    // debug("Connected to IBUS daemon for IME input management\n");
}

static DBusHandlerResult
ibus_on_owner_change(DBusConnection* conn, DBusMessage* msg, void* user_data) {
    if (dbus_message_is_signal(msg, "org.freedesktop.DBus", "NameOwnerChanged")) {
        const char* name;
        const char* old_owner;
        const char* new_owner;

        if (!dbus_message_get_args(msg, NULL,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_STRING, &old_owner,
            DBUS_TYPE_STRING, &new_owner,
            DBUS_TYPE_INVALID
        )) {
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }

        if (strcmp(name, "org.freedesktop.IBus") != 0) {
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }

        GLFWIBUSData* ibus = (GLFWIBUSData*) user_data;
        ibus->name_owner_changed = true;

        return DBUS_HANDLER_RESULT_HANDLED;

    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

 bool LeditIBus::readAddress() {
    static char buf[1024];
    struct stat s;
    FILE *addr_file = fopen(ibusData.address_file_name.c_str(), "r");
    if (!addr_file) {
        // _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to open IBUS address file: %s with error: %s", ibus->address_file_name, strerror(errno));
        return false;
    }
    int stat_result = fstat(fileno(addr_file), &s);
    bool found = false;
    while (fgets(buf, sizeof(buf), addr_file)) {
        if (strncmp(buf, "IBUS_ADDRESS=", sizeof("IBUS_ADDRESS=")-1) == 0) {
            size_t sz = strlen(buf);
            if (buf[sz-1] == '\n') buf[sz-1] = 0;
            if (buf[sz-2] == '\r') buf[sz-2] = 0;
            found = true;
            break;
        }
    }
    fclose(addr_file); 
    addr_file = NULL;
    if (stat_result != 0) {
        // _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to stat IBUS address file: %s with error: %s", ibus->address_file_name, strerror(errno));
        return false;
    }
    ibusData.address_file_mtime = s.st_mtime;
    if (found) {
        ibusData.address = std::string(buf + sizeof("IBUS_ADDRESS=") - 1);
        return true;
    }
    // _glfwInputError(GLFW_PLATFORM_ERROR, "Could not find IBUS_ADDRESS in %s", ibus->address_file_name);
    return false;
}
LeditIBus::LeditIBus() {
	dBus.connect();
}
void LeditIBus::setFocused(bool focused) {
	if(!connected)
		return;
    ibusData.conn->callMethodNoReply(IBUS_SERVICE, ibusData.input_ctx_path.c_str(), IBUS_INPUT_INTERFACE, focused ? "FocusIn" : "FocusOut", DBUS_TYPE_INVALID);

}
void LeditIBus::setGeometry(int x, int y, int w, int h) {
		if(!connected)
		return;
	  ibusData.conn->callMethodNoReply(IBUS_SERVICE, ibusData.input_ctx_path.c_str(), IBUS_INPUT_INTERFACE, "SetCursorLocation",
                DBUS_TYPE_INT32, &x, DBUS_TYPE_INT32, &y, DBUS_TYPE_INT32, &w, DBUS_TYPE_INT32, &h, DBUS_TYPE_INVALID);

}
bool LeditIBus::sendKey(int key, int scancode, int mods, int action) {
  if(!connected)
      return false;
  uint32_t state = keyStateFromGlfw(mods, action);

    if (!ibusData.conn->callMethodWithReply(
            IBUS_SERVICE, ibusData.input_ctx_path.c_str(), IBUS_INPUT_INTERFACE, "ProcessKeyEvent",
            3000,  [&](DBusMessage *msg, const char* err, void* data) {
            	uint32_t handled = 0;
            	if(!err){
            		ibusData.conn->getArgs(msg, "Failed to get IBUS handled key from reply", DBUS_TYPE_BOOLEAN, &handled, DBUS_TYPE_INVALID);

            	}
            	std::cout << "handled: " << handled << "\n";
            }, NULL,
            DBUS_TYPE_UINT32, &scancode, DBUS_TYPE_UINT32, &key, DBUS_TYPE_UINT32,
            &state, DBUS_TYPE_INVALID)) {
      
        return false;
    }
    return true;
}
uint32_t LeditIBus::keyStateFromGlfw(unsigned int glfw_modifiers, int action) {
    uint32_t ans = action == GLFW_RELEASE ? IBUS_RELEASE_MASK : 0;
#define M(g, i) if(glfw_modifiers & GLFW_MOD_##g) ans |= i
    M(SHIFT, IBUS_SHIFT_MASK);
    M(CAPS_LOCK, IBUS_LOCK_MASK);
    M(CONTROL, IBUS_CONTROL_MASK);
    M(ALT, IBUS_MOD1_MASK);
    M(NUM_LOCK, IBUS_MOD2_MASK);
    M(SUPER, IBUS_MOD4_MASK);
    /* To do: figure out how to get super/hyper/meta */
#undef M
    return ans;
}
void LeditIBus::dispatch(){
    if(!connected)
        return;
    ibusData.conn->dispatch();
}