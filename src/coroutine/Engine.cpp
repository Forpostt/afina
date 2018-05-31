#include <afina/coroutine/Engine.h>

#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <cstring>

namespace Afina {
namespace Coroutine {

void Engine::Store(context &ctx) {
    char cur;
    ctx.Low = std::min(&cur, StackBottom);
    ctx.Hight = std::max(&cur, StackBottom);

    size_t size = ctx.Hight - ctx.Low;

    if (size > std::get<1>(ctx.Stack)) {
        delete[] std::get<0>(ctx.Stack);
        std::get<0>(ctx.Stack) = new char[size];
        std::get<1>(ctx.Stack) = size;
    }
    std::memcpy(std::get<0>(ctx.Stack), ctx.Low, size);
}

void Engine::Restore(context &ctx) {
    char cur;

    while (ctx.Low <= &cur && &cur <= ctx.Hight)
        Restore(ctx);

    std::memcpy(ctx.Low, std::get<0>(ctx.Stack), ctx.Hight - ctx.Low);
    longjmp(ctx.Environment, 1);
}

void Engine::yield() {
    auto new_routine = alive;

    while (new_routine == cur_routine && new_routine)
        new_routine = new_routine->next;

    sched(new_routine);
}

void Engine::sched(void *routine_) {
    auto routine = (context *)routine_;

    if(!routine || routine == cur_routine){
        return;
    }

    if (cur_routine) {
        Store(*cur_routine);
        if (setjmp(cur_routine->Environment)) {
            return;
        }
    }
    cur_routine = routine;
    Restore(*routine);
}



} // namespace Coroutine
} // namespace Afina
