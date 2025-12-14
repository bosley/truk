# Vision: Process-Level Concurrency in Truk

    An AI generated this when discussing my "vision" so i didnt have to write the whole thing out. since this shit is woo-woo anyway it doesnt matter how accurate it is

## The Problem

Modern concurrent programming typically follows one of two paths:

1. **Lightweight concurrency** (Go, Erlang) - abstractions that sit below OS processes
2. **Heavy isolation** (traditional multiprocessing) - full OS processes with complex IPC

Neither approach is ideal for all use cases. Lightweight concurrency sacrifices isolation for performance, while heavy isolation sacrifices ergonomics for safety.

## The Truk Approach: Outward Concurrency

### Going Outward vs. Going Inward

**Go went inward** - making concurrency lighter by going below the OS:
- Goroutines are cheaper than threads
- Managed by runtime, not OS
- Share address space by default
- Fast, but crash in one goroutine can affect others

**Truk goes outward** - making parallelism safer by embracing OS processes:
- Full process isolation
- OS-managed scheduling
- Shared memory is opt-in and scoped
- Slower spawn, but true fault isolation

### Build Fingerprinting for Type Safety

The key innovation is using build fingerprints to enable safe inter-process communication:

```
Build Hash = SHA256(source_files + dependencies + compiler_flags)
```

When multiple instances of the same build run:
- They share the same struct layouts
- They share the same type definitions
- They can safely share memory without serialization
- The fingerprint proves structural compatibility

This gives us **zero-copy IPC with compile-time type safety guarantees**, something traditional multiprocessing doesn't provide.

## Comparison to Existing Systems

### vs. Go Goroutines

**Go:**
- Lightweight: thousands of goroutines per process
- Shared memory by default
- Panic in one can crash all
- Single runtime managing everything

**Truk:**
- Heavyweight: OS processes
- Isolated by default
- Crash in one doesn't affect others
- OS manages processes independently

**Trade-off:** Go is faster to spawn and coordinate. Truk is more robust and truly parallel.

### vs. Erlang Processes

**Erlang:**
- Lightweight processes with message passing
- "Let it crash" philosophy
- Built for distributed systems
- VM-managed

**Truk:**
- OS processes with shared memory
- "Let it crash" philosophy (inherited from process isolation)
- Built for multi-core systems
- OS-managed

**Trade-off:** Erlang is better for distributed systems. Truk is better for single-machine parallelism.

### vs. Traditional Multiprocessing

**Traditional (Python, C++):**
- Full process isolation
- Complex IPC (pipes, sockets, serialization)
- Manual coordination
- No type safety across boundaries

**Truk:**
- Full process isolation
- Simple shared memory (when fingerprints match)
- Runtime-assisted coordination
- Type safety via fingerprinting

**Trade-off:** Traditional approaches are more flexible. Truk is safer and more ergonomic.

## Ephemeral Shared Memory

A key insight of the fingerprint-scoped approach is that shared memory has a natural lifecycle tied to process presence:

### Collective Persistence

When multiple processes with the same fingerprint run:
- First process creates the shared memory region
- Subsequent processes attach to the existing region
- Memory persists as long as **any** process is attached
- Last process to exit triggers cleanup

This creates an interesting property: **memory exists as long as someone remembers it**.

### Example Lifecycle

```
T0: Process A starts → creates shared memory, writes data
T1: Process B starts → attaches to memory, reads A's data, writes more
T2: Process A crashes → memory persists (B still attached)
T3: Process C starts → attaches to memory, sees A+B's data
T4: Process B exits → memory persists (C still attached)
T5: Process C exits → memory is freed (no processes remain)
```

### Properties

**Emergent Durability:**
- No explicit persistence mechanism required
- Data lives as long as the collective needs it
- Naturally garbage collected when no longer in use
- No orphaned resources or cleanup required

**Fault Tolerance:**
- Any single process can crash without data loss
- Knowledge accumulates across process lifetimes
- New processes inherit accumulated state
- System degrades gracefully as processes exit

**Coordination Without Central Authority:**
- No master process or coordinator needed
- Peer-to-peer memory sharing
- Democratic lifecycle management
- Self-organizing system

### Use Cases

**Accumulated State:**
A fleet of worker processes can build up shared state over time. As long as at least one worker remains, the accumulated work persists. New workers can join and immediately access all previous results.

**Transient Caching:**
Multiple instances of a service can share a cache. The cache exists as long as any instance is serving requests. When load drops to zero, the cache naturally disappears.

**Collaborative Processing:**
Processes can contribute partial results to shared memory. The complete result emerges from collective effort. If some processes fail, others continue and the partial work is preserved.
