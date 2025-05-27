#include "marker.h"


DWORD WINAPI marker(LPVOID marker_data) {
    MarkerData* data = static_cast<MarkerData*>(marker_data);

    WaitForSingleObject(data->startEvent, INFINITE);

    srand(data->marker_index);
    std::vector<int> marked_ind;
    int num_marked = 0;

    while (true) {

        int x = rand() % data->size;

        EnterCriticalSection(&(data->cs));
        if (data->arr[x] == 0) {

            Sleep(5);
            data->arr[x] = data->marker_index;
            Sleep(5);
            num_marked++;
            marked_ind.push_back(x);

            LeaveCriticalSection(&(data->cs));
        }
        else {
            std::cout << "I am marker number " << data->marker_index << '\n'
                << "I marked " << num_marked << " elements\n"
                << "I cannot mark element with index " << x << '\n';

            std::cout << '\n';
            SetEvent(data->stopEvent);

            WaitForSingleObject(data->resumeEvent, INFINITE);
            ResetEvent(data->resumeEvent);

            DWORD res = WaitForSingleObject(data->exitEvent, 0);

            if (res == WAIT_OBJECT_0) {
                for (int i = 0; i < marked_ind.size(); i++) {
                    data->arr[marked_ind[i]] = 0;
                }
                LeaveCriticalSection(&(data->cs));
                return 0;
            }
            LeaveCriticalSection(&(data->cs));
        }

    }
    return 0;
}
