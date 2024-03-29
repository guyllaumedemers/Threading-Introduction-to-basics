# Threading-Introduction-to-basics

### Using multithreading and why it matters.

**Wikipedia** : "`Multithreading` is the ability of a central processing unit (CPU) (or a single core in a multi-core processor) to provide multiple threads of execution concurrently, supported by the operating system."

#### Project

Example details are grouped into subject related points and showcase investigation result on the subject of software configuration management and version control. Further details will be provided on more advance features that are covered in the making of our examples.

#### What this README.md is not

Be aware that some of the tooling used in the making of this `Demo` project won't be covered here. External documentations will be provided for your own benefit, which in most case, are also where most of the information mentioned here will be coming from.

## What's a Thread

**Wikipedia** : "A `thread` of execution is the smallest sequence of programmed instructions that can be managed independently by a scheduler, which is typically a part of the operating system."

## What's a Mutex

**Wikipedia** : "A `lock` or `mutex` (from [mutual exclusion](https://en.wikipedia.org/wiki/Mutual_exclusion)) is a synchronization primitive that prevents state from being modified or accessed by multiple threads of execution at once."

## What's an atomic type

**Cppreference** : "Objects of `atomic` types are the only objects that are free from data races; that is, they may be modified by two threads concurrently or modified by one and read by another."

# Building with multithreading in mind

### Mutex-based

```
// example of a simple concurrent application using lock-based mechanism (Mutex)

Simple example	-	3x Coworkers start the day with their coffee cup full. The office coffee machine also start full.

			During the day, they each empty their cup and go to the machine for a refill. Only one person can refill
			at a time. Each person, drinking at about the same pace.

Goal		-	Which of the three coworker will have to refill the machine?

```

##### Example - MSVC Development Toolkit Cmd-line

```
C:> cl /std:c++17 Main.cc		(Main.obj, Main.exe)
```

**Hint** : Example location [path](https://github.com/guyllaumedemers/Threading-Introduction-to-basics/blob/master/simple-concurrency/Main.cc).

## More examples coming soon!...