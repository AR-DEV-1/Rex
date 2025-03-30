# Rex Globals

## Introduction
Everyone knows that globals should get avoided in software development, however sometimes there isn't really an alternative. Therefore Rex has come up with a design strategy to initialize, use and destroy globals. It is highly recommended that anyone who needs to write their own globals uses this same strategy

## Setup
The setup for a new global is fairly straightforward. The public API looks like this

```
// The class of which an instance will be a global
// The class's public interface will be the interface of the global as well
class MyGlobalClass
{

};

namespace my_global
{
    // Initialize the global from a unique ptr.
    // Because we pass in the global, the user has control of its creation
    // making it easier when dealing with initialization order of globals
    void init(rsl::unique_ptr<MyGlobalClass> myGlobal);

    // This func returns the pointer of the unique pointer passed in above
    // Returning a pointer to an instance of the class allows us
    // to use the class' public API to be exposed to the user
    MyGlobalClass* instance();

    // Reset the pointer passed in the init function, freeing up all the memory
    void shutdown();
}
```