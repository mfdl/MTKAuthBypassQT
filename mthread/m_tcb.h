#ifndef TCB_H
#define TCB_H

class TCB
{
public:
    virtual void Execute() = 0;
    virtual ~TCB(){}

    void set_arg(void *arg) { usr_arg = arg; }
    void * get_arg() { return usr_arg; }

private:
    void* usr_arg;
};

template <class Receiver>
class MCB : public TCB
{
public:
    typedef void (Receiver::* Action)();

    MCB(Receiver* r, Action a)
        : _receiver(r), _action(a) { }

    virtual void Execute();

private:
    Receiver* _receiver;
    Action _action;
};

template <class Receiver>
void MCB<Receiver>::Execute ()
{
    (_receiver->*_action)();
}

#endif // TCB_H
