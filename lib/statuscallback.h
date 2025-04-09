#ifndef STATUS_CALLBACK_H
#define STATUS_CALLBACK_H

using StatusCallback = void (*)(const char*);

inline void emptyStatus(const char*) {}

#endif // STATUS_CALLBACK_H