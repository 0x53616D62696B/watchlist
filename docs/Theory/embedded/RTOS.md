# RTOS Tutorial for Embedded C++ Programming

## What Is an RTOS?

An RTOS, or Real-Time Operating System, is a small operating system designed for systems that must react to events within predictable timing limits.

In embedded programming, an RTOS lets you split firmware into multiple tasks instead of writing one large `while (true)` loop. Each task can handle one responsibility, such as reading sensors, updating a display, processing communication, or controlling a motor.

The important part is not that an RTOS is "fast". The important part is that it is predictable.

## Real-Time Does Not Mean Instant

"Real-time" means the system has timing requirements.

- A soft real-time system should respond quickly, but an occasional delay is acceptable.
- A firm real-time system loses value if it misses a deadline, but failure is not catastrophic.
- A hard real-time system must meet deadlines, or the system can become unsafe or invalid.

Examples:

- Soft real-time: updating a UI, logging sensor data, streaming telemetry.
- Firm real-time: dropping an old video frame if it is too late.
- Hard real-time: airbag control, pacemaker timing, motor safety shutdown.

## Bare-Metal vs RTOS

### Bare-Metal Firmware

Bare-metal firmware usually runs directly on the microcontroller without an operating system.

A common structure looks like this:

```cpp
int main()
{
    initializeHardware();

    while (true)
    {
        readSensors();
        updateControlLoop();
        handleCommunication();
        updateDisplay();
    }
}
```

This is simple and often excellent for small systems.

### RTOS Firmware

With an RTOS, work is split into tasks:

```cpp
void sensorTask(void* argument);
void controlTask(void* argument);
void communicationTask(void* argument);
void displayTask(void* argument);

int main()
{
    initializeHardware();

    createTask(sensorTask);
    createTask(controlTask);
    createTask(communicationTask);
    createTask(displayTask);

    startScheduler();

    while (true)
    {
        // Usually never reached.
    }
}
```

The RTOS scheduler decides which task runs at a given time.

## Core RTOS Concepts

### Task

A task is like a small independent program with its own stack.

Typical tasks:

- Sensor sampling task
- Motor control task
- Communication task
- Display/UI task
- Logging task
- Watchdog task

Each task usually runs forever:

```cpp
void sensorTask(void*)
{
    while (true)
    {
        readSensor();
        delayMilliseconds(10);
    }
}
```

### Scheduler

The scheduler decides which task runs.

Most embedded RTOS schedulers use priorities:

- Higher priority tasks run before lower priority tasks.
- A blocked task does not consume CPU time.
- A task can block while waiting for time, data, a mutex, or an interrupt event.

### Context Switch

A context switch happens when the RTOS stops one task and resumes another.

This has overhead. The CPU must save registers, switch stacks, and restore another task's state.

Context switching is useful, but it is not free.

### Priority

Priority controls scheduling importance.

Example:

- High priority: motor safety shutdown
- Medium priority: sensor processing
- Low priority: logging debug messages

Priority should represent timing importance, not personal preference.

### Tick

Many RTOS kernels use a periodic timer interrupt called the system tick.

The tick lets the RTOS track time, wake delayed tasks, and perform scheduling decisions.

Some RTOSes also support tickless idle, where the periodic tick is stopped during low-power sleep.

## Common RTOS Primitives

### Delay

A task delay pauses a task without blocking the entire CPU.

```cpp
while (true)
{
    sampleTemperature();
    vTaskDelay(pdMS_TO_TICKS(100));
}
```

This is different from a busy wait:

```cpp
while (timerNotExpired())
{
    // Wastes CPU.
}
```

Prefer RTOS delays when the task has nothing useful to do.

### Queue

A queue passes data from one task to another safely.

Example:

- Sensor task reads a value.
- Sensor task sends the value to a queue.
- Processing task receives the value later.

```cpp
struct SensorSample
{
    int temperature;
    int humidity;
};
```

Queues are one of the cleanest ways to communicate between tasks.

### Mutex

A mutex protects shared resources.

Use it when multiple tasks access the same object, peripheral, or data structure.

Examples:

- Shared I2C bus
- Shared SPI bus
- Shared log buffer
- Shared configuration object

```cpp
lock(mutex);
writeToSharedResource();
unlock(mutex);
```

Keep mutex-protected sections short.

### Semaphore

A semaphore is a signaling primitive.

Common uses:

- Notify a task that an interrupt occurred.
- Count available resources.
- Signal that a buffer is ready.

Binary semaphores are often used as event signals.

Counting semaphores are useful when there can be multiple pending events or resources.

### Event Flags

Event flags allow a task to wait for one or more events.

Example:

- Bit 0: Wi-Fi connected
- Bit 1: time synchronized
- Bit 2: configuration loaded

A task can wait until all required bits are set.

### Software Timer

A software timer runs a callback after a time interval.

Use software timers for lightweight timing events. Avoid doing heavy work in timer callbacks.

## Why Use an RTOS?

Use an RTOS when your firmware has multiple independent jobs with different timing needs.

Good reasons:

- You need predictable task scheduling.
- You have communication, sensors, control, UI, and logging running together.
- Some operations block, such as waiting for a message or peripheral.
- You need clear separation of responsibilities.
- You want to avoid a giant fragile main loop.
- You need synchronization primitives like queues, semaphores, and mutexes.
- You are using middleware that expects an RTOS, such as networking stacks or USB stacks.

## When Not to Use an RTOS

Do not add an RTOS just because it sounds more professional.

Avoid an RTOS when:

- The firmware is very small.
- A simple state machine is enough.
- RAM is extremely limited.
- Timing is simple and fully controlled by interrupts.
- The team is not ready to debug concurrency problems.
- Certification or safety constraints make the extra kernel complexity undesirable.

For many embedded projects, bare-metal code with interrupts and state machines is still the best design.

## Pros

- Cleaner separation between firmware responsibilities.
- Easier to handle blocking operations.
- Better structure for complex systems.
- Built-in synchronization primitives.
- Priority-based scheduling for time-sensitive work.
- Easier integration with networking, filesystems, USB, BLE, and other middleware.
- Can improve responsiveness when designed well.
- Helps isolate slow tasks from urgent tasks.

## Cons

- More RAM usage because each task needs a stack.
- More flash usage because the RTOS kernel is included.
- Context switch overhead.
- More complex debugging.
- Race conditions become possible.
- Deadlocks become possible.
- Priority inversion can occur.
- Bad task priorities can make the system unreliable.
- Timing can become harder to reason about if tasks are poorly designed.

## RTOS in C++: Important Considerations

Most embedded RTOS APIs are C APIs. C++ works well with them, but you need a clean boundary.

### Use Static or Free Function Task Entrypoints

Most RTOS task functions look like this:

```cpp
void taskFunction(void* argument);
```

A non-static member function does not match that signature because it has an implicit `this` pointer.

Use a static trampoline:

```cpp
class SensorService
{
public:
    void run()
    {
        while (true)
        {
            readSensor();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }

    static void taskEntry(void* argument)
    {
        auto* self = static_cast<SensorService*>(argument);
        self->run();
    }

private:
    void readSensor()
    {
        // Read hardware here.
    }
};
```

Task creation:

```cpp
SensorService sensorService;

xTaskCreate(
    SensorService::taskEntry,
    "sensor",
    512,
    &sensorService,
    2,
    nullptr
);
```

### Be Careful With Dynamic Allocation

Many embedded C++ projects avoid heap allocation after startup.

Reasons:

- Fragmentation
- Allocation failure
- Non-deterministic allocation time
- Harder failure analysis

Prefer:

- Static allocation
- Fixed-size buffers
- Object pools
- RTOS static task creation APIs where available

### Avoid Exceptions Unless Your Platform Supports Them Well

C++ exceptions can increase code size and complicate embedded failure behavior.

Many embedded projects disable exceptions and RTTI.

If exceptions are disabled, use explicit error handling:

```cpp
enum class Result
{
    Ok,
    Timeout,
    BusError
};
```

### RAII Is Still Useful

RAII can make mutex handling safer.

```cpp
class MutexLock
{
public:
    explicit MutexLock(SemaphoreHandle_t mutex)
        : mutex_(mutex)
    {
        xSemaphoreTake(mutex_, portMAX_DELAY);
    }

    ~MutexLock()
    {
        xSemaphoreGive(mutex_);
    }

private:
    SemaphoreHandle_t mutex_;
};
```

Usage:

```cpp
void writeLog(const char* message)
{
    MutexLock lock(logMutex);
    lowLevelWrite(message);
}
```

RAII helps avoid forgetting to unlock a mutex when returning early.

## Example: Simple FreeRTOS-Style C++ Design

This example shows a sensor task sending data to a processing task.

```cpp
#include <cstdint>

struct SensorSample
{
    std::uint32_t timestampMs;
    int rawValue;
};

QueueHandle_t sensorQueue;

class SensorReader
{
public:
    void run()
    {
        while (true)
        {
            SensorSample sample {
                getMilliseconds(),
                readAdc()
            };

            xQueueSend(sensorQueue, &sample, pdMS_TO_TICKS(10));
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }

    static void taskEntry(void* argument)
    {
        static_cast<SensorReader*>(argument)->run();
    }

private:
    std::uint32_t getMilliseconds()
    {
        return xTaskGetTickCount() * portTICK_PERIOD_MS;
    }

    int readAdc()
    {
        // Replace with real ADC read.
        return 0;
    }
};

class SensorProcessor
{
public:
    void run()
    {
        SensorSample sample {};

        while (true)
        {
            if (xQueueReceive(sensorQueue, &sample, portMAX_DELAY) == pdTRUE)
            {
                process(sample);
            }
        }
    }

    static void taskEntry(void* argument)
    {
        static_cast<SensorProcessor*>(argument)->run();
    }

private:
    void process(const SensorSample& sample)
    {
        // Filter, validate, or forward the sample.
    }
};

SensorReader sensorReader;
SensorProcessor sensorProcessor;

int main()
{
    initializeHardware();

    sensorQueue = xQueueCreate(8, sizeof(SensorSample));

    xTaskCreate(SensorReader::taskEntry, "sensor", 512, &sensorReader, 2, nullptr);
    xTaskCreate(SensorProcessor::taskEntry, "processor", 768, &sensorProcessor, 2, nullptr);

    vTaskStartScheduler();

    while (true)
    {
    }
}
```

Notes:

- The sensor task samples every 20 ms.
- The queue decouples sampling from processing.
- The processor blocks until data is available.
- Neither task needs to know how the other task is scheduled.

## Designing RTOS Tasks

Good task design:

- Each task has one clear responsibility.
- Tasks spend most of their time blocked, not spinning.
- Shared state is minimized.
- Queues are preferred over global variables.
- Interrupts do minimal work and notify tasks.
- Priorities are chosen based on deadlines.
- Stack sizes are measured, not guessed forever.

Poor task design:

- Too many tasks.
- Every task has the same priority without thought.
- Tasks busy-wait in loops.
- Large objects are placed on small task stacks.
- Multiple tasks modify global state directly.
- Long mutex holds block high-priority work.

## Interrupts and RTOS Tasks

Interrupts should be short.

Good interrupt behavior:

1. Clear the hardware interrupt flag.
2. Capture minimal data if needed.
3. Notify a task.
4. Exit quickly.

Example pattern:

```cpp
void uartInterruptHandler()
{
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    readByteFromUart();
    xSemaphoreGiveFromISR(uartSemaphore, &higherPriorityTaskWoken);

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}
```

The task does the heavier work:

```cpp
void uartTask(void*)
{
    while (true)
    {
        xSemaphoreTake(uartSemaphore, portMAX_DELAY);
        processReceivedBytes();
    }
}
```

## Priority Inversion

Priority inversion happens when a high-priority task waits for a low-priority task that owns a shared resource.

Example:

1. Low-priority task locks an I2C mutex.
2. High-priority task needs the same I2C mutex and blocks.
3. Medium-priority task runs and prevents the low-priority task from releasing the mutex.
4. The high-priority task is indirectly blocked by the medium-priority task.

Many RTOS mutexes support priority inheritance, where the low-priority task temporarily inherits the higher priority until it releases the mutex.

Use proper RTOS mutexes, not binary semaphores, when protecting shared resources.

For a fuller explanation, examples, and prevention checklist, see [Priority Inversion in RTOS](../../docs/Theory/priority_inversion.md).

## Deadlocks

A deadlock occurs when tasks wait forever for each other.

Example:

- Task A locks mutex 1, then waits for mutex 2.
- Task B locks mutex 2, then waits for mutex 1.

Prevention:

- Always acquire locks in the same order.
- Keep lock duration short.
- Avoid calling unknown code while holding a mutex.
- Use timeouts where appropriate.
- Prefer message passing over shared locks.

## Stack Size

Each task has its own stack.

Watch for:

- Large local arrays
- Deep function calls
- Recursive functions
- Heavy C++ objects on the stack
- Library calls with unknown stack usage

Prefer static buffers or carefully owned long-lived objects for large data.

Many RTOSes provide stack high-water-mark APIs. Use them during testing.

## Memory Usage

An RTOS consumes memory for:

- Kernel data
- Task control blocks
- Task stacks
- Queues
- Semaphores
- Mutexes
- Timers

Before choosing an RTOS, check:

- RAM size
- Flash size
- Required task count
- Worst-case stack usage
- Middleware memory needs

On very small MCUs, memory cost may be the deciding factor.

## Common RTOS Options

### FreeRTOS

FreeRTOS is widely used, small, and common on microcontrollers.

Good for:

- STM32
- ESP32
- NXP
- Microchip
- Many Cortex-M systems

### Zephyr

Zephyr is a larger embedded RTOS with device drivers, networking, build tooling, and board definitions.

Good for:

- More integrated platform support
- Networking
- Bluetooth
- Device tree based hardware configuration
- Projects that benefit from a larger ecosystem

### ThreadX / Azure RTOS

ThreadX is a commercial-grade RTOS historically used in professional embedded systems.

Good for:

- Products that need vendor support
- Systems already integrated with ThreadX middleware

### CMSIS-RTOS

CMSIS-RTOS is not exactly a kernel by itself. It is an API abstraction layer used on Arm Cortex-M systems.

It can sit on top of kernels such as FreeRTOS or RTX.

## RTOS Use Cases

### Good Fit

- Motor controller with communication and safety monitoring.
- IoT device with sensors, Wi-Fi, MQTT, and OTA update logic.
- Data logger with filesystem, USB, and periodic sampling.
- Robot firmware with control loops, telemetry, and command handling.
- Product with UI, buttons, display, and background communication.

### Usually Overkill

- Single LED blinker.
- Simple sensor readout.
- Small one-purpose device with no blocking operations.
- Firmware that can be cleanly represented as one state machine.

## Typical Embedded C++ RTOS Architecture

A practical design might look like this:

```text
Application
  SensorService
  ControlService
  CommunicationService
  LoggingService

Drivers
  Gpio
  SpiBus
  I2cBus
  Uart
  Adc

Platform
  RTOS wrappers
  clock/timer
  board startup
```

Keep hardware drivers separate from application tasks when possible.

Example:

- `SensorService` owns the sensor task.
- `I2cBus` owns the low-level I2C transaction logic.
- A mutex protects `I2cBus` if multiple services share it.

## Practical Rules of Thumb

- Start with the fewest tasks that make the design clear.
- Prefer queues for task communication.
- Prefer mutexes only when sharing cannot be avoided.
- Keep interrupt handlers tiny.
- Avoid busy waiting.
- Measure stack usage.
- Make task priorities boring and intentional.
- Avoid dynamic allocation after startup.
- Do not call blocking RTOS APIs from interrupt context unless they are explicitly ISR-safe.
- Use timeouts to make failures visible.
- Build watchdog behavior into the design.

## A Simple Decision Checklist

Use an RTOS if most of these are true:

- The firmware has several independent responsibilities.
- Some work has stricter timing than other work.
- Communication or middleware can block.
- The main loop is becoming hard to maintain.
- You need queues, semaphores, mutexes, or timers.
- You have enough RAM for task stacks.
- The team can test and debug concurrent code.

Stay bare-metal if most of these are true:

- The firmware has one primary job.
- Timing is simple.
- RAM is very limited.
- A state machine is clear and maintainable.
- Interrupts handle the few urgent events.
- You do not need blocking middleware.

## Learning Path

1. Write a simple bare-metal loop first.
2. Learn interrupts and timers.
3. Learn cooperative state machines.
4. Learn RTOS tasks and delays.
5. Add queues between tasks.
6. Add mutexes only when needed.
7. Learn ISR-to-task notification.
8. Measure stack usage and timing.
9. Practice debugging race conditions and deadlocks.
10. Build a small project with sensors, communication, and logging.

## Mini Project Ideas

### Sensor Logger

Tasks:

- Sensor task samples every 100 ms.
- Logger task writes samples to a ring buffer.
- Communication task sends data over UART.

Concepts:

- Queues
- Periodic tasks
- Shared buffer protection

### Button-Controlled LED Controller

Tasks:

- Button task waits for button events.
- LED task changes blink pattern.
- Status task prints debug information.

Concepts:

- Interrupt-to-task notification
- Event flags
- Task delays

### Motor Safety Monitor

Tasks:

- Control loop task runs periodically.
- Safety task monitors limits.
- Communication task receives commands.

Concepts:

- Priorities
- Watchdog design
- Deadline thinking

## Summary

An RTOS is useful when embedded firmware has multiple concurrent responsibilities and predictable timing requirements. It gives you tasks, scheduling, queues, semaphores, mutexes, timers, and better structure for complex systems.

The tradeoff is complexity. You gain powerful tools, but you must handle concurrency, memory usage, task priorities, stack sizing, deadlocks, and race conditions carefully.

For embedded C++, an RTOS can work very well when you keep the C API boundary clean, use static task entry functions, avoid unnecessary dynamic allocation, and design tasks around message passing instead of shared global state.

The best rule: use an RTOS when it simplifies the system's real behavior, not when it merely makes the firmware look more advanced.

## Sources and Further Reading

These references were used as background material for the RTOS concepts in this document:

- [FreeRTOS kernel documentation: tasks and co-routines](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/00-Tasks-and-co-routines): task structure, scheduling concepts, and FreeRTOS task APIs.
- [FreeRTOS kernel documentation: queues](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/02-Queues-mutexes-and-semaphores/01-Queues): queue-based communication between tasks.
- [Zephyr Project documentation: threads](https://docs.zephyrproject.org/latest/kernel/services/threads/index.html): thread lifecycle, stacks, priorities, scheduling behavior, and thread states.
- [Arm CMSIS-RTOS2 documentation](https://arm-software.github.io/CMSIS_6/latest/RTOS2/index.html): RTOS API abstraction for Arm Cortex-M systems.
- [Eclipse ThreadX repository](https://github.com/eclipse-threadx/threadx): ThreadX background, positioning, and source distribution.
- [Zephyr Project documentation: kernel services](https://docs.zephyrproject.org/latest/kernel/services/index.html): synchronization, data passing, timing, and memory-management concepts.
