# RedisGears Functions

A RedisGears **function** is a formal description of the processing steps in the data flow.

```
                                      +------------+
                                      | Function   |
                    +-------------+   | +--------+ |
                    | Input data  +-->+ | Reader | |
                    +-------------+   | +---+----+ |
                                      |     v      |
                                      | +---+----+ |
                                      | | Step 1 | |
                                      | +---+----+ |
                                      |     |      |
                                      |    ...     |
                                      |     v      |
                                      | +---+----+ |
                                      | | Step n | |
                                      | +---+----+ |
                                      |     v      |
                    +-------------+   | +---+----+ |
                    | Results     +<--+ | Action | |
                    +-------------+   | +--------+ |
                                      +------------+
```

A function always:

  1. Starts with a [**reader**](readers.md)
  2. Operates on zero or more [**records**](glossary.md#record)
  3. Consists of zero or more [**operations**](operations.md) (a.k.a steps)
  4. Ends with an [**action**](glossary.md#action)
  5. Returns zero or more [**results**](glossary.md#result)
  6. May generate zero or more errors

## Execution
A function is executed by the RedisGears engine in one of two ways:

  * **Batch**: execution is immediate and on existing data
  * **Event**: execution is triggered by new events and on their data

The function's mode of execution is determined by its action. There are two types of actions:

  * [**Run**](#run): runs the function in batch
  * [**Register**](#register): registers the function to be triggered by events

When executed, whether as batch or event, the function's context is managed by the engine. Besides the function's logic, the context also includes its breakdown to internal execution steps, status, statistics, results and any errors encountered (among other things).

!!! note "Related commands"
    The following RedisGears commands are related to executing functions:

    * [`RG.PYEXECUTE`](commands.md#rgpyexecute)
    * [`RG.ABORTEXECUTION`](commands.md#rgabortexecution)
    * [`RG.DUMPEXECUTIONS`](commands.md#rgdumpexecutions)
    * [`RG.GETEXECUTION`](commands.md#rggetexecution)

## Execution ID
The execution of every function is internally assigned a unique value called the **Execution ID**.

The ID is a string made up of two parts and delimited by a hyphen ('-'):

  1. **Shard ID**: the 40-bytes-long identifier of a [shard](glossary.md#shard) in a [cluster](glossary.md#cluster)
  2. **Sequence**: an ever-increasing counter

!!! example "Example: Execution IDs"
    When used in _stand-alone_ mode, the Shard ID is set to zeros ('0'), so the first execution ID would be:

    ```
    0000000000000000000000000000000000000000-1
    ```

    Whereas in _cluster_ mode, the execution ID might be:

    ```
    a007297351b007297351c007297351d007297351-1
    ```

## Execution Plan
Before executing any function, the engine generates an **Execution Plan**. The plan consists of the basic steps that the engine will take to execute the function.

## Execution Parallelization
When run in a cluster, the execution plan is generated by the _initiator_. The execution plan is then shared and executed in parallel across all shards, by default. The initiator's coordinator orchestrates the distributed operation.

## Execution Status
The **Execution Status** describes the function's current execution status. The status will be one of the following:

* **created**: the execution has been created
* **running**: the execution is running
* **done**: the execution is done
* **aborted**: the execution has been aborted
* **pending_cluster**: the initiator is waiting for all workers to finish
* **pending_run**: worker is pending OK from initiator to execute
* **pending_receive**: the initiator is pending acknowledgement from workers on receiving execution
* **pending_termination**: worker is pending a termination messaging from the initiator

The following diagram illustrates the relevant state transitions:

```
              Initiator                                  Worker
       +---------------------+  execution plan   +---------------------+
       |             created +------------------>+ created             |
       +----------+----------+                   +----------+----------+
                  v                                         v
       +----------+----------+  acknowledgement  +----------+----------+
       |     pending_receive +<------------------+ pending_run         |
       +----------+----------+                   +---------------------+
                  v
       +----------+----------+  start execution  +---------------------+
       |             running +------------------>+ running             |
       +----------+----------+                   +----------+----------+
                  v                                         v
       +----------+----------+      results      +----------+----------+
       |     pending_cluster +<------------------+ pending_termination |
       +----------+----------+                   +---------------------+
                  v
       +----------+----------+     terminate     +---------------------+
       |                done +------------------>+ done                |
       +---------------------+                   +---------------------+
```

## Registration
The representation of an event-driven function is called a registration.

Registrations are persisted in Redis snapshots (that is, RDB files). This allows recovery of both data and event handlers in the event of a failure.

!!! note "Related commands"
    The following RedisGears commands are related to registering functions:

    * [`RG.PYEXECUTE`](commands.md#rgpyexecute)
    * [`RG.DUMPREGISTRATIONS`](commands.md#rgdumpregistrations)
    * [`RG.UNREGISTER`](commands.md#rgunregister)

## Registration ID
Every registration has a unique internal identifier called a **Registration ID**. This ID is generated in the same manner as the [Execution ID](#execution-id). Despite their similar appearance, the two should not be confused.

## Context Builder
RedisGears functions in Python always begin with a context builder: the [`#!python class GearsBuilder`](runtime.md#gearsbuilder).

!!! tip
    `GB()` is an alias for `GearsBuilder()`.

    This alias is intended to be used for brevity, increased productivity, and the reduction of finger strain due to repetitive typing.

**Python API**
```python
class GearsBuilder(reader='KeysReader', defaultArg='*', desc=None)
```
_Arguments_

* _reader_: the function's [reader](readers.md)
* _defaultArg_: Optional arguments that the reader may need. These are usually a key's name, prefix, glob, or a regular expression. Its use depends on the function's reader type and action.
* _desc_: an optional description

**Examples**
```python
# Here's how to run the default context builder:
GearsBuilder().run()

# You can also do this:
gb = GB()
gb.register()
```

## Actions
An action is a special type of operation that terminates a function. The current supported actions are `run()` and `register()`.

### Run
The **Run** action runs a function as a batch job. In this case, the function will be executed once and then exit as soon as the data is exhausted by its reader.

Trying to run more than one function in the same execution will fail with an error.

!!! example "Example: multiple executions error"
    ```
    127.0.0.1:30001> RG.PYEXECUTE "GB().run()\nGB().run()"
    (error) [... 'spam.error: Can not run more then 1 executions in a single script']
    ```

!!! important "Execution is always asynchronous"
    Batch functions are **always executed asynchronously** by the RedisGears engine. That means that they are run in a background thread, not by the main Redis server thread.

**Python API**
```python
class GearsBuilder.run(arg=None, convertToStr=True, collect=True)
```

_Arguments_

* _arg_: An optional argument that's passed to the reader as its _defaultArg_. It means the following:
    * A glob-like pattern for the [KeysReader](readers.md#keysreader) and [KeysOnlyReader](readers.md#keysonlyreader) readers
    * A key name for the [StreamReader](readers.md#streamreader) reader
    * A Python generator for the [PythonReader](readers.md#pythonreader) reader
* _convertToStr_: when `True` adds a [map](operations.md#map) operation to the flow's end that converts records to strings
* _collect_: when `True` adds a [collect](operations.md#collect) operation to flow's end

**Examples**
```python
{{ include('functions/run.py') }}
```

### Register
The **Register** action registers a function as an event handler. The function is executed each time an event arrives. Each time it is executed, the function operates on the event's data and once done is suspended until new events invoke it again.

**Python API**
```python
class GearsBuilder.register(convertToStr=True, collect=True, mode='async', onRegistered=None)
```

_Arguments_

* _convertToStr_: when `True` adds a [map](operations.md#map) operation to the flow's end that converts records to strings
* _collect_: when `True` adds a [collect](operations.md#collect) operation to flow's end

* _mode_: the execution mode of the triggered function. Can be one of:
    * **'async'**: execution will be asynchronous across the entire cluster
    * **'async_local'**: execution will be asynchronous and restricted to the handling shard
    * **'sync'**: execution will be synchronous and local
* _onRegistered_: A function [callback](operations.md#callback) that's called on each shard upon function registration. It is a good place to initialize non-serializable objects such as network connections.

Notice that you can pass more arguments `register()` function, but those arguments depend on the reader. You can read about these additional arguments on the [readers](readers.md) page.

**Examples**
```python
{{ include('functions/register.py') }}
```

## Results
The execution of a function yields zero or more **result** records. The result is made up of any records output by the function's last operation and just before its final action.

Results are stored in the function's execution context.

!!! note "Related commands"
    The following RedisGears commands are related to getting results:

    * [`RG.GETRESULTS`](commands.md#rggetresults)
    * [`RG.GETRESULTSBLOCKING`](commands.md#rggetresultsblocking)
