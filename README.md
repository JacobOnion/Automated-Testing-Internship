# Automated Testing Projects

Three tools focused on improving security when grading student code.

## Graphics-Autograder

A low-level autograder than can be used in place of any currently used autograder in Gradescope. The focus of this autograder is on identifying potential security concerns in the student code before the marker runs the submission locally on their machine for futher marking.

### Dependencies

Requires podman to be available for local testing and pushing autograder changes to repository.

### Setup and execution

To build and run locally:
```
podman build -t autograder:latest .
podman run autograder:latest
```

To push to the Dockerhub Repository:
1. make an account on Dockerhub

2. Set up a repository

3. Run these commands:
```
* podman login docker.io -u 'username'
* podman tag autograder:latest docker.io/'username'/'repository name':latest
* podman push docker.io/'username'/'repository name':latest
```

4. In Gradescope, when setting the autograder, select manual docker configuration and write 'username'/'repository name':latest


## Syscall filter Constructor

A tool to build a syscall whitelist based on an example valid submission, to then use on any student submissions. Can be run locally outside of a container.

### Setup and execution

1. Add the example submission to the "Pristine" directory

2. build the executables:
```
./build_project.sh
```

3.  Run:
```
build/RunPristine
```

This should create a file called "syscalls.txt", containing all the syscall numbers used in the pristine code.

4. Student code can be added to the local Submission directory, or run from any directory by specifying the path as a command line argument.
   * Run in local submission directory:
   ```
   build/RunSubmission
    ```
   * Run in specified directory:
   ```
   build/RunSubmission 'absolute path to submission directory'
   ```


## Local Execution Container

A local podman container that can be used to run suspicious code safely. Makes use of a ptrace harness and seccomp filter to prevent code escaping the container.

### Dependencies

Requires podman to be available to run the container.

### Setup and execution

1. Replace the contents of the OpenGL directory with the student code.
   Ensure glad, ScreenGrabLib.cpp and stb_image_write.h exist and the directory root.

2. Run these commands:
```
podman build -t container:latest .
podman run container:latest
```


## Help

For any questions about these tools, email jacobeonion@gmail.com

## Future work

* These tools may be more useful if the user specifies the path to the student code as a command line argument, should be easy enough to implement.

* Dockerhub repository is currently public, as allowing Gradescope to access a private repository requires a paid account. Options are to pay for this, or look into a free alternative such as GitLab Container Registery.