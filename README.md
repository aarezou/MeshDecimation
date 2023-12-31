# MeshDecimation

**This code is an implementation of Quadric-based mesh decimation via multiple choices, written as an assignment for CMPT 764 course at SFU.**

In short, the goal is to simplify the input mesh by collapsing n edges of it. To choose the edge for each iteration, k edges are randomly chosen and the quadric error metric is calculated for each edge, finally, the edge with the minimum error is chosen to be collapsed. When collapsed, the edge is replaced with a vertex and the neighbors are updated. In this implementation, the mesh is stored in a [winged-edge](https://en.wikipedia.org/wiki/Winged_edge) data structure.

The following explanation is borrowed from the assignment defenition:

    You are to implement a mesh decimation algorithm driven by the quadric-based errors, where the outer optimization is implemented using the multiple choice scheme. The input mesh is assumed to be a *connected, closed manifold triangle mesh*. Thus there is no need to implement vertex pair collapse, just implement edge collapse.
    The original mesh simplification paper using quadric-based error metrics is here http://mgarland.org/files/papers/quadrics.pdf.
    The paper on the use of the multiple-choice scheme is here http://www.graphics.rwth-aachen.de/media/papers/mcd_vmv021.pdf.

    Multiple choice scheme: Instead of relying on a priority queue which enables us to choose the edge collapse that would introduce the least quadric error, select the edge collapse amongst k randomly chosen candidate edges which gives the least quadric error.

    Inner optimization: Find the best vertex location which minimizes the quadric error.

    Assignment of quadrics: As explained in the lecture slides, initially, each vertex stores a sum of quadrics of the supporting planes of its incident triangles. The quadric associated with an edge (u, v) is the sum of quadrics at u and v. Similarly, when an edge (u, v) is collapsed to w, then the quadric assigned to w is the sum of quadrics at u and v.


## How to Run the Code
Clone the repository using the --recursive flag to download the necessary submodules:

    git clone https://github.com/aarezou/MeshDecimation.git --recursive

Then, run the following commands to compile the code:

    cd MeshDecimation
    mkdir build
    cd build
    cmake ..
    make

Finally run the applicaion:

    ./mcaq
