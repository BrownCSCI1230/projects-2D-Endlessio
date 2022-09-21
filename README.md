# Projects 1 & 2: Brush & Filter

All project handouts can be found [here](https://browncsci1230.github.io/projects).



# Part 1: Brush

## Introduction

Computer graphics often deals with digital images, which are two-dimensional arrays of color data (the pixels on the screen). In this project, a painting application that features multiple types of brushes that can be used to paint on a 2D canvas, similar to applications such as MS Paint and Photoshop.



## Principle Data Structure

### Color

a struct contains variables including red color, green color, blue color and alpha represented in unsigned 8 bit integer.

### Canvas

given canvas size of 1D vector, the element type of the vector is RGBA color struct

### Brush

a 1D vector mask with element be float with the size be brush size

### Prev Color for Smudge

given brush size of 1D vector, the element type of the vector is RGBA color struct

### Init Color (my fun exploration for eraser)

a struct of color contains the init canvas color same as when initialize/clear canvas

### Previous Canvas (my fun exploration for Undo Button)

double-ended queue (deque) structure with element be vector of color (canvas vector) with manually defined max_depth meaning the number of previous canvas we memorize.

the deque structure is chosen because we follow a "LRU" base to evict the oldest restored canvas if the current prev_canvas_queue size exceeds the max_depth before we push the new canvas into the queue.





## Principle Function Implementation

### Basic Part

- Distance Check: the rule author followed here is first use Euclidean Rule calculate two point distance, after rounding the distance, if the distance is smaller than or equal to brush radius, then inlcude that point (mark with certain value in the mask)

  ![截屏2022-09-20 下午10.08.58](/Users/endlessio/CS1230/projects-2D-Endlessio/report_images/截屏2022-09-20 下午10.08.58.png)

- Constant Brush: if the distance is <= brush radius, fill the vector index with 1.0.

  

- Linear Brush: the equation for the linear brush is $$1-\frac{distance}{r}$$, if the distance is <= brush radius, fill the vector index with result calculated from the linear equation.

- Quadratic Brush: after calculation, $$A = \frac{1}{r^2}, B = -\frac{2}{r}, C = 1$$, so the equation for the quadratic brush is $$(\frac{1.0}{r^2})\times distance^2-\frac{2.0}{r}\times distance+1$$, if the distance is <= brush radius, fill the vector index with result calculated from the quadratic equation.

- Smudge Brush: smudge is actually similar to other brushes, the critical difference is it does not use the brush color, instead it uses the previous color we store in the vector. The process is when mouse down,  write the previous color vector according to the current brush parameters; then mouse drag, blend the previous color and current canvas color, update the previous color vector to current blended color. For the brush mask, the writer choose the quadratic brush since it is the most "soft" one, which is good for the blend effect.

### Extra Credit Part

- Fill Bucket

  - the basic idea is when mouse down and settings is fill bucket, we obtain the start color. Then we starting from the start color, iterating the canvas with DFS or BFS. The iteration keeps going when the current color is **same as the start color** and **the current position is not being visited**. It stops when encountering visited position or cross canvas boundary or current color is not the same as the start color

  - the writer chooses BFS here for two reasons: (1) better for later feature, if we want to realize tolerance for the fill bucket later, BFS is intuitively more reasonabke. (2) the implementation of BFS is queue maintained by the developer, thus will not encounter the overflow problem compared to the recursion version of DFS (of course we can also manually maintain a stack for DFS to avoid exceed max recuision problem)

    ![截屏2022-09-20 下午10.07.15](/Users/endlessio/CS1230/projects-2D-Endlessio/report_images/截屏2022-09-20 下午10.07.15.png)

- Spray

  - the writer uses random function to generate random number and then remainder by 100, which makes the range of the result be a random number between 0 to 100. 

  - then since density is also in the range of 0 to 100, we can see density (with certain proportion) as a threshold.

    ![截屏2022-09-20 下午10.06.49](/Users/endlessio/CS1230/projects-2D-Endlessio/report_images/截屏2022-09-20 下午10.06.49.png)

  - then when using the for loop to go through the brush mask (being set to 1 if in distance), we apply the random function above to see if it is a hit (over threshold), if it is a hit, we do not show the pixel by overriding the mask intensity to 0 (you can also implement the reverse version)

    ![截屏2022-09-20 下午10.06.35](/Users/endlessio/CS1230/projects-2D-Endlessio/report_images/截屏2022-09-20 下午10.06.35.png)

### My Fun Exploration Part

- Eraser

  - this one is trivial, we just need to record the "paper color", the color we use in the background

  - then when chosing eraser option, we just rewrite the part to the paper color

  - the brush type can be constant ("hard" eraser), can also to linear ("soft" eraser)

    ![截屏2022-09-20 下午10.06.54](/Users/endlessio/CS1230/projects-2D-Endlessio/report_images/截屏2022-09-20 下午10.06.54.png)

- Undo One Stroke

  - sometimes we want to undo our canvas, so we need to store the previous canvas status. Okay...so what  would be a proper frequency to store it? naturally, we can store every stroke.

  - now the question is how to detect one stroke, simple, it is just when the mouse is up. One stroke contains mouse down (start) -> mouse drag (continue) -> mouse down (end).

  - cool, now the problem is trade-off between experience and efficiency: do we want to store every stroke from the beginning? it will provide user more accessibility but it has large space complexity. So here, the writer chose to make the max depth be 5, that is, when the queue length is larger than max depth, we pop out the oldest one (popright) in the deque, and push the new one (pushleft)

    ![截屏2022-09-20 下午10.11.57](/Users/endlessio/CS1230/projects-2D-Endlessio/report_images/截屏2022-09-20 下午10.11.57.png)

  - so the whole process is when mouse up, push the current canvas into deque, when undo buttom is clicked, pop the most recent canvas data (popleft)

    ![截屏2022-09-20 下午10.12.23](/Users/endlessio/CS1230/projects-2D-Endlessio/report_images/截屏2022-09-20 下午10.12.23.png)



