# EventEmitter / EventLoop
## Nomenclature TDB

## About
Take a look at test/generic_test.cpp for now.

## Warning
I can't guarantee that this doesn't leak all kinds of memory.  In fact, I can guarantee that Dispatching an event to storage doesn't clear up if the thread ends.

I'll keep working on it.

## Using
```c++
class MyObject : public EventEmitter::Emitter {}

auto object = new MyObject();

object->On(EventEmitter::EventId(1), []() {
	// do something asynchronously
}, EventEmitter::Async);

object->On(EventEmitter::EventId(2), []() {
	// do something synchronously!
}, EventEmitter::Immediate);

object->On(EventEmitter::EventId(3), [](){
	// dispatch this function to the thread that called me!
}, EventEmitter::Dispatch);

object->Emit(EventEmitter::EventId(1)); // invoked with std::async
object->Emit(EventEmitter::EventId(2)); // Blocks until complete
object->Emit(EventEmitter::EventId(3)); // Nothing's happened yet?!
EventEmitter::EventLoop::ProcessEvents(); // now EventId(3) fires.
```

## Why?  There's like a hundred libraries that do this already
I like to lear by doing and I wanted to learn more about some async concepts and type-erasure.