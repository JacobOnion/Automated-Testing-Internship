#FROM gradescope/autograder-base
FROM ubuntu:22.04
#FROM gcc:16


#ADD setup.sh /gradescope

RUN mkdir -p /LocalCont/source


# Trying to get base ubuntu image to work (added python3 for different base image)
RUN apt-get clean && rm -rf /var/lib/apt/lists/* && apt-get update && apt-get install -y dos2unix python3 && rm -rf /var/lib/apt/lists/*

ADD source /LocalCont/source

#RUN mkdir -p /autograder/results

RUN cp /LocalCont/source/main.sh /LocalCont/main.sh

RUN chmod 755 /LocalCont/main.sh

RUN dos2unix /LocalCont/main.sh  /LocalCont/source/setup.sh
#RUN chmod +x /autograder/run_autograder


# Compile code in image so environment is consistent
#RUN bash /autograder/source/setup.sh && \
#    make -C /autograder/source -f ContainerTests.make && \
#    make -C /autograder/source -f OpenGLProject.make && \
#    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN bash /LocalCont/source/setup.sh


RUN make -C /LocalCont/source -f OpenGLProject.make

RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

#RUN apt update && apt upgrade -y

ENTRYPOINT ["/LocalCont/main.sh"]
#ENTRYPOINT [ "/LocalCont/source/build/autograder/RunSubmission" ]