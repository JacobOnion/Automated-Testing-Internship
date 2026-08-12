# Automated Testing Projects

3 tools focused on improving security when grading student code.

## Graphics-Autograder

A low-level autograder than can be used in place of any currently used autograder in Gradescope.

### Dependencies

Requires podman to be available for local testing and pushing to repository.

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

### Dependencies


### Setup and execution

1. Add the example submission to the "Pristine" directory

2. Run these Commands:
```
* ./premake5 gmake
* make
* build/RunPristine
```

This should create a file called "syscalls.txt", containing all the syscall numbers used in the pristine code.

3. Run:
   ```
   build/RunSubmission
    ```


## Local Execution Container

A local podman container that can be used to run suspicious code safely. Makes use of a ptrace harness and seccomp filter to prevent code escaping the container.

### Dependencies

Requires podman to be available to run the container.

### Setup and execution

1. Add the student code to the ___ directory

2. Run these commands:
```
podman build -t container:latest .
podman run container:latest
```


## Help

Any advise for common problems or issues.
```
command to run if program contains helper info
```

## Acknowledgments

Inspiration, code snippets, etc.
* [awesome-readme](https://github.com/matiassingers/awesome-readme)
* [PurpleBooth](https://gist.github.com/PurpleBooth/109311bb0361f32d87a2)
* [dbader](https://github.com/dbader/readme-template)
* [zenorocha](https://gist.github.com/zenorocha/4526327)
* [fvcproductions](https://gist.github.com/fvcproductions/1bfc2d4aecb01a834b46)
