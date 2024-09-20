#include "dbus.h"
#include <iostream>

static dbus_bool_t
add_dbus_watch(DBusWatch *watch, void *data) {
    // id_type watch_id = addWatch(dbus_data->eld, data, dbus_watch_get_unix_fd(watch), events_for_watch(watch), dbus_watch_get_enabled(watch), on_dbus_watch_ready, watch);
    // if (!watch_id) return FALSE;
    // id_type *idp = malloc(sizeof(id_type));
    // if (!idp) return FALSE;
    // *idp = watch_id;
    // dbus_watch_set_data(watch, idp, free);
    return TRUE;
}

static void
remove_dbus_watch(DBusWatch *watch, void *data) {
    // id_type *idp = dbus_watch_get_data(watch);
    // if (idp) removeWatch(dbus_data->eld, *idp);
}

static void
toggle_dbus_watch(DBusWatch *watch, void *data) {
    // id_type *idp = dbus_watch_get_data(watch);
    // if (idp) toggleWatch(dbus_data->eld, *idp, dbus_watch_get_enabled(watch));
}

static dbus_bool_t
add_dbus_timeout(DBusTimeout *timeout, void *data) {
    // int enabled = dbus_timeout_get_enabled(timeout) ? 1 : 0;
    // monotonic_t interval = ms_to_monotonic_t(dbus_timeout_get_interval(timeout));
    // if (interval < 0) return FALSE;
    // id_type timer_id = addTimer(dbus_data->eld, data, interval, enabled, true, on_dbus_timer_ready, timeout, NULL);
    // if (!timer_id) return FALSE;
    // id_type *idp = malloc(sizeof(id_type));
    // if (!idp) {
    //     removeTimer(dbus_data->eld, timer_id);
    //     return FALSE;
    // }
    // *idp = timer_id;
    // dbus_timeout_set_data(timeout, idp, free);
    return TRUE;

}

static void
remove_dbus_timeout(DBusTimeout *timeout, void *data) {
    // id_type *idp = dbus_timeout_get_data(timeout);
    // if (idp) removeTimer(dbus_data->eld, *idp);
}

static void
toggle_dbus_timeout(DBusTimeout *timeout, void *data) {
    // id_type *idp = dbus_timeout_get_data(timeout);
    // if (idp) toggleTimer(dbus_data->eld, *idp, dbus_timeout_get_enabled(timeout));
}
static const char*
format_message_error(DBusError *err) {
    static char buf[1024];
    snprintf(buf, sizeof(buf), "[%s] %s", err->name ? err->name : "", err->message);
    return buf;
}
static void
method_reply_received(DBusPendingCall *pending, void *user_data) {
    MethodResponse *res = (MethodResponse*)user_data;
    RAII_MSG(msg, dbus_pending_call_steal_reply(pending));
    if (msg) {
        DBusError err;
        dbus_error_init(&err);
        if (dbus_set_error_from_message(&err, msg)) (*res->cb)(NULL, format_message_error(&err), NULL);
        else (*res->cb)(msg, NULL, NULL);
    }
}

void dbus_g_callback(DBusMessage *msg, const char* err, void* data) {

}
LeditDBusConnection* LeditDBus::connectTo(const char *path, const char* err_msg, const char *name, bool register_on_bus) {
	std::string name_str(name);
	if(this->connections.count(name_str))
		return this->connections[name_str];
    DBusError err;
    dbus_error_init(&err);
    DBusConnection *ans = dbus_connection_open_private(path, &err);
    if (!ans) {
    	std::cout << "DBus Connection is null\n";
        // report_error(&err, err_msg);
        return NULL;
    }
    dbus_connection_set_exit_on_disconnect(ans, FALSE);
    dbus_error_free(&err);
    if (register_on_bus) {
        if (!dbus_bus_register(ans, &err)) {
          	  // report_error(&err, err_msg);

    	std::cout << "dbus_bus_register is null\n";
            return NULL;
        }
    }
    if (!dbus_connection_set_watch_functions(ans, add_dbus_watch, remove_dbus_watch, toggle_dbus_watch, (void*)name, NULL)) {
        // _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to set DBUS watches on connection to: %s", path);
        dbus_connection_close(ans);
        dbus_connection_unref(ans);
        return NULL;
    }
    if (!dbus_connection_set_timeout_functions(ans, add_dbus_timeout, remove_dbus_timeout, toggle_dbus_timeout, (void*)name, NULL)) {
        // _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to set DBUS timeout functions on connection to: %s", path);
        dbus_connection_close(ans);
        dbus_connection_unref(ans);
        return NULL;
    }
    LeditDBusConnection* conn = new LeditDBusConnection(ans);
    this->connections[name_str] = conn;
    return conn;
}
void LeditDBus::dispatch() {
	if (session_bus) {
		  while(dbus_connection_dispatch(session_bus) == DBUS_DISPATCH_DATA_REMAINS);
	}
}
void LeditDBus::connect() {
	if(this->session_bus)
		return;
	DBusConnection *session_bus = this->session_bus;
	    DBusError error;
    dbus_error_init(&error);
    if (session_bus) {
        dbus_connection_unref(session_bus);
    }
    session_bus = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if (dbus_error_is_set(&error)) {
        // report_error(&error, "Failed to connect to DBUS session bus");
        session_bus = NULL;
        return;
    }
    static const char *name = "session-bus";
    if (!dbus_connection_set_watch_functions(session_bus, add_dbus_watch, remove_dbus_watch, toggle_dbus_watch, (void*)name, NULL)) {
        // _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to set DBUS watches on connection to: %s", name);
        dbus_connection_close(session_bus);
        dbus_connection_unref(session_bus);
        return;
    }
    if (!dbus_connection_set_timeout_functions(session_bus, add_dbus_timeout, remove_dbus_timeout, toggle_dbus_timeout, (void*)name, NULL)) {
        // _glfwInputError(GLFW_PLATFORM_ERROR, "Failed to set DBUS timeout functions on connection to: %s", name);
        dbus_connection_close(session_bus);
        dbus_connection_unref(session_bus);
        return;
    }
    std::cout << "connected to dbus!\n";
}

bool LeditDBusConnection::callMethodWithMsgCb(DBusMessage *msg, int timeout, void *user_data, const MessageCallback& cb) {
	bool retval = false;
	  DBusPendingCall *pending = NULL;
      if (dbus_connection_send_with_reply(connection, msg, &pending, timeout)) {
      	MethodResponse* res = new MethodResponse;
      	res->cb = &cb;
        dbus_pending_call_set_notify(pending, method_reply_received, res, free);
     } else{
     	return false;
     }
	return true;
}
bool LeditDBusConnection::callMethodWithMsg(DBusMessage *msg) {
	bool retval = false;
	  DBusPendingCall *pending = NULL;
      if (!dbus_connection_send(connection, msg, NULL)) {
      	return false;
     }
	return true;
}
bool LeditDBusConnection::callMethodCb(const char *node, const char *path, const char *interface, const char *method, int timeout, const MessageCallback& cb, void *user_data, va_list ap) {
    std::cout << "Path: " << path << "\n";
    RAII_MSG(msg, dbus_message_new_method_call(node, path, interface, method));
    if (!msg) return false;
    bool retval = false;

    int firstarg = va_arg(ap, int);
    if ((firstarg == DBUS_TYPE_INVALID) || dbus_message_append_args_valist(msg, firstarg, ap)) {
        retval = callMethodWithMsgCb(msg, timeout, user_data, cb);
    } else {
    	return false;
    }
    return retval;
}
bool LeditDBusConnection::callMethod(const char *node, const char *path, const char *interface, const char *method, int timeout, void *user_data, va_list ap) {
 RAII_MSG(msg, dbus_message_new_method_call(node, path, interface, method));
    if (!msg) return false;
    bool retval = false;

    int firstarg = va_arg(ap, int);
    if ((firstarg == DBUS_TYPE_INVALID) || dbus_message_append_args_valist(msg, firstarg, ap)) {
        retval = callMethodWithMsg(msg);
    } else {
    	return false;
    }
    return retval;
}
bool LeditDBusConnection::callMethodWithReply(const char *node, const char *path, const char *interface, const char *method, int timeout, const MessageCallback& cb, void* user_data, ...) {
    bool retval;
    va_list ap;
    va_start(ap, user_data);
    retval = callMethodCb(node, path, interface, method, timeout, cb, user_data, ap);
    va_end(ap);
    return retval;
}

bool LeditDBusConnection::callMethodNoReply(const char *node, const char *path, const char *interface, const char *method, ...) {
    bool retval;
    va_list ap;
    va_start(ap, method);
    retval = callMethod(node, path, interface, method, DBUS_TIMEOUT_USE_DEFAULT, NULL, ap);
    va_end(ap);
    return retval;
}

bool LeditDBusConnection::getArgs(DBusMessage *msg, const char *failmsg, ...) {
    DBusError err;
    dbus_error_init(&err);
    va_list ap;
    va_start(ap, failmsg);
    int firstarg = va_arg(ap, int);
    bool ret = dbus_message_get_args_valist(msg, &err, firstarg, ap) ? true : false;
    va_end(ap);
    if (!ret) {
    	return false;
    }
    return ret;
}
void LeditDBusConnection::dispatch(){
     while(dbus_connection_dispatch(connection) == DBUS_DISPATCH_DATA_REMAINS);
}