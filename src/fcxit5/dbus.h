#ifndef LEDIT_DBUS_H
#define LEDIT_DBUS_H

#include <dbus/dbus.h>
#include <unordered_map>
#include <string>
#include <functional>

static inline void cleanup_msg(void *p) { DBusMessage *m = *(DBusMessage**)p; if (m) dbus_message_unref(m); m = NULL; }
#define RAII_MSG(name, initializer) __attribute__((cleanup(cleanup_msg))) DBusMessage *name = initializer


using MessageCallback = std::function<void(DBusMessage *msg, const char* err, void* data)>;
extern "C" {

	static dbus_bool_t
add_dbus_watch(DBusWatch *watch, void *data);
static void
remove_dbus_watch(DBusWatch *watch, void *data);
static void
toggle_dbus_watch(DBusWatch *watch, void *data);
static dbus_bool_t
add_dbus_timeout(DBusTimeout *timeout, void *data);
static void
remove_dbus_timeout(DBusTimeout *timeout, void *data);
static void
toggle_dbus_timeout(DBusTimeout *timeout, void *data);
static void
method_reply_received(DBusPendingCall *pending, void *user_data);
static const char*
format_message_error(DBusError *err);
}
struct MethodResponse {
	const MessageCallback* cb;
};
class LeditDBusConnection {
private:
	DBusConnection *connection = nullptr;

public:
	LeditDBusConnection(DBusConnection *connection_) : connection(connection_) {}
	bool callMethodWithMsgCb(DBusMessage *msg, int timeout, void *user_data, const MessageCallback& cb);
	bool callMethodWithMsg(DBusMessage *msg);
	bool callMethodCb(const char *node, const char *path, const char *interface, const char *method, int timeout, const MessageCallback& cb, void *user_data, va_list ap);
	bool callMethodWithReply(const char *node, const char *path, const char *interface, const char *method, int timeout, const MessageCallback& cb, void* user_data, ...);
	bool callMethod(const char *node, const char *path, const char *interface, const char *method, int timeout, void *user_data, va_list ap);
	bool callMethodNoReply(const char *node, const char *path, const char *interface, const char *method, ...);
	bool getArgs(DBusMessage *msg, const char *failmsg, ...);
	void dispatch();
	DBusConnection* conn() {
		return connection;
	};
};
class LeditDBus {
private:
	DBusConnection *session_bus = nullptr;
	std::unordered_map<std::string, LeditDBusConnection*> connections;
public:
	void connect();

LeditDBusConnection* connectTo(const char *path, const char* err_msg, const char *name, bool register_on_bus);
void dispatch();

};
#endif