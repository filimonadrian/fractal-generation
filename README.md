# Fractal-generation

## How to run  

---

- run parallel implementation: 
  - from ./sol directory: `make && ./tema1_par <input_file_julia> <output_file_julia> <intput_file_mandelbrot> <output_file_mandelbrot> <number_of_threads>`
- run sequential implementation
  - from ./skel directory `make && ./tema1 <input_file_julia> <output_file_julia> <intput_file_mandelbrot> <output_file_mandelbrot> <number_of_threads>`
- to compare the sequential and the parallel resuls:
  - from ./sol directory: `./test.sh`

## Implementation details

---

- **main** function:
  - initialization of the **barrier**
  - thread creation
  - thead join

- **generate_images** function:
  - this is where the magic happens
  - the function receives as parameter **thread_id**
  - if there is just one thread, all functions (read, allocate, write, free space) are executed sequentially
  - if there are at least 2 theads the function uses this pattern: 
    - **thread 0** *read the file* for the julia_algorithm, *allocate* the *result matrix* and *initialize* the variables
    - all threads will run *julia algorithm* and the *tranformation of coordinates*
    - after calculus, **thread 0** writes the **result in file* and *frees the allocated memory*
    - while the **thread 0** finish his job, the **<P - 1> thread** *reads the input file* for the mandelbrot algorithm, *allocate the result matrix* and *initializes the variables*
    - all threads will run *mandelbrot algorithm* and the *tranformation of coordinates*  
    - **<P - 1> thread** will *write the result* in the output file and *free the memory*
- **transform_coordinates** function:
  - takes the *result matrix* which contains the mathematical coordinates and transform themn in screen coordinates
- **run_julia** and **run_mandelbrot**
  - calculate the *start* and *stop* indices for every thread and uses them in the second for
