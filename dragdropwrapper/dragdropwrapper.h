#ifndef DRAG_DROP_H
#define DRAG_DROP_H

#include <functional>

extern "C" {
    void SendFileAsDragDrop(void* handle, const char* file_path, std::function<void(void)> callback);
}

#endif // DRAG_DROP_H

