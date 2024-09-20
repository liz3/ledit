#ifndef LEDIT_IBUS_H
#define LEDIT_IBUS_H

#include "dbus.h"
#include <cstring>
#include <string>
#include <cstdint>


struct GLFWIBUSData {
    bool ok, inited, name_owner_changed;
    time_t address_file_mtime;
    LeditDBusConnection *conn = nullptr;
    std::string input_ctx_path, address_file_name, address;
};



#define PATH_MAX 4096
static const char IBUS_SERVICE[]         = "org.freedesktop.IBus";
static const char IBUS_PATH[]            = "/org/freedesktop/IBus";
static const char IBUS_INTERFACE[]       = "org.freedesktop.IBus";
static const char IBUS_INPUT_INTERFACE[] = "org.freedesktop.IBus.InputContext";
enum Capabilities {
    IBUS_CAP_PREEDIT_TEXT       = 1 << 0,
    IBUS_CAP_AUXILIARY_TEXT     = 1 << 1,
    IBUS_CAP_LOOKUP_TABLE       = 1 << 2,
    IBUS_CAP_FOCUS              = 1 << 3,
    IBUS_CAP_PROPERTY           = 1 << 4,
    IBUS_CAP_SURROUNDING_TEXT   = 1 << 5
};

typedef enum
{
    IBUS_SHIFT_MASK    = 1 << 0,
    IBUS_LOCK_MASK     = 1 << 1,
    IBUS_CONTROL_MASK  = 1 << 2,
    IBUS_MOD1_MASK     = 1 << 3,
    IBUS_MOD2_MASK     = 1 << 4,
    IBUS_MOD3_MASK     = 1 << 5,
    IBUS_MOD4_MASK     = 1 << 6,
    IBUS_MOD5_MASK     = 1 << 7,
    IBUS_BUTTON1_MASK  = 1 << 8,
    IBUS_BUTTON2_MASK  = 1 << 9,
    IBUS_BUTTON3_MASK  = 1 << 10,
    IBUS_BUTTON4_MASK  = 1 << 11,
    IBUS_BUTTON5_MASK  = 1 << 12,

    /* The next few modifiers are used by XKB, so we skip to the end.
     * Bits 15 - 23 are currently unused. Bit 29 is used internally.
     */

    /* ibus mask */
    IBUS_HANDLED_MASK  = 1 << 24,
    IBUS_FORWARD_MASK  = 1 << 25,
    IBUS_IGNORED_MASK  = IBUS_FORWARD_MASK,

    IBUS_SUPER_MASK    = 1 << 26,
    IBUS_HYPER_MASK    = 1 << 27,
    IBUS_META_MASK     = 1 << 28,

    IBUS_RELEASE_MASK  = 1 << 30,

    IBUS_MODIFIER_MASK = 0x5f001fff
} IBusModifierType;

extern "C" {
	static const char*
get_ibus_address_file_name(void);
static DBusHandlerResult
ibus_on_owner_change(DBusConnection* conn, DBusMessage* msg, void* user_data);
static DBusHandlerResult
message_handler(DBusConnection *conn, DBusMessage *msg, void *user_data);
}

class LeditIBus {
private:
	LeditDBus dBus;
	bool connected = false;
	GLFWIBUSData ibusData;
	bool readAddress();
	int state = 0;
	void onInputContextCreated(DBusMessage *msg);
	uint32_t keyStateFromGlfw(unsigned int glfw_modifiers, int action);
public:
	LeditIBus();
	void connect();
    void dispatch();
	void setFocused(bool focused);
	void setGeometry(int x, int y, int w, int h);
	bool sendKey(int key, int scancode, int mods, int action);
};
#endif