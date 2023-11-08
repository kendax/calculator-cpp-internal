# Using this as the base image
FROM drogonframework/drogon::latest

# Copying all project files to the container's /usr/src/drogo directory
COPY . /usr/src/drogo

# Declaring the working directory inside the container
WORKDIR /usr/src/drogo/build

# Running the program's build commands
RUN cmake .. && make 

# Set an environment variable for port in the container
ENV PORT "8080"

# Open up port 8080 in the container to use for the program
EXPOSE 8080

# Command to run the program
CMD ["./calculator_cpp_internal"]