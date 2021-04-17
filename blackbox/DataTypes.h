#ifndef __DATATYPES_H__
#define __DATATYPES_H__

// TODO: find out why bb doesn't use the STL and if there is no good reason, replace this with an STL implementation..
template<typename T>
struct StackQueue {
private:
    struct List {
        struct List* prev;
        struct List* next;
        T* item;

        List(List* prev, List *next, T* item) : prev(prev), next(next), item(item) {}
    };

    List* head;
    List* tail;
    unsigned int count;

    void SetFirst(List* headTail) {
        this->head = headTail;
        this->tail = headTail;
        this->count = 1;
    }

    T* Unlink(List* list) {
        T* item = list->item;

        if(list->prev != NULL)
            list->prev->next = list->next;

        if(list->next != NULL)
            list->next->prev = list->prev;

        if(list == this->head)
            this->head = list->next;

        if(list == this->tail)
            this->tail = list->prev;

        this->count--;
        delete list;
        return item;
    }

public:
    StackQueue() : head(NULL), tail(NULL), count(0) {}

    ~StackQueue() {
        while(this->head) {
            Unlink(this->head);
        }
    }

    unsigned int GetCount() {
        return count;
    }

    void PushHead(T* item) {
        if(this->head == NULL) {
            SetFirst(new List(NULL, NULL, item));
            return;
        }

        this->head->prev = new List(NULL, this->head, item);
        this->head = this->head->prev;
        this->count++;
    }

    void PushTail(T* item) {
        if(this->tail == NULL) {
            SetFirst(new List(NULL, NULL, item));
            return;
        }

        this->tail->next = new List(this->tail, NULL, item);
        this->tail = this->tail->next;
        this->count++;
    }

    T* PopHead() {
        if(this->head == NULL)
            return NULL;

        return Unlink(this->head);
    }

    T* PopTail() {
        if(tail == NULL)
            return NULL;

        return Unlink(this->tail);
    }
    
    bool Remove(T* item) {
        List* found = NULL;
        List* search = this->head;

        while(search != NULL) {
            if(search->item != item) {
                search = search->next;
                continue;
            }

            found = search;
            search = search->next;
            this->Unlink(found);
        }

        return !!found;
    }
};

#endif