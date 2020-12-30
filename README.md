# Comparing-Different-Pathfinding-Algorithms

## What is Path finding?

**Path finding** in applications is the research of finding the shortest and/or cheapest (optimal) route between 2 points, usually on graphs. To achieve this, many different algorithms are used, all having better performance in certain situations. These are heavily based on **Dijkstra's algorithm**, which is an algorithm Created by **Edsger W. Dijkstra** in 1956 to find the shortest path between 2 **nodes** on a graph. This algorithm gets used in, for example, modern GPS systems to find the shortest path to get to your destination. In this Research project, I used the framework provided by my college (**DAE Howest**) to compare 2 well known path finding algorithms, and 1 lesser known algorithm that is a variant on 1 of these 2, to find the most optimal one. These algorithms being:
* Breadth First Search (BFS)
* A Star (A*)
* Jump Point Search (JPS)

BFS and A* I had to implement during my classes Gameplay programming, so this research will mostly be focusing on the implementation of JPS.

## Color code for the algorithms

- **Bright Green Circle**: Start node
- **Bright Red Circle**: End node
- **Darker Green Circles**: Final path between the 2 nodes
- **Red Rectangles**: Closed list of the algorithm
- **Yellow Rectangles**: Open list of the algorithm
- **Grey Rectangles**: unvisited nodes of the graph
- **Brown Rectangles**: Mud nodes (more expensive nodes to go through)
- **Blue Rectangles**: Water nodes (nodes that are impossible to go through)

## Implementation

As part of my classes, I got an example application that loads a grid with nodes and connections between these nodes for us. 

### Breadth First Search

The hands on for this application was the programming of the BFS algorithm. This algorithm first searches all the neighbors of the starting node, then continues to all the neighbors of these neighbors, and so on until it finds a clear shortest path from the start to the end node. This algorithm is slow, but it guarantees the shortest path.

![Image example of BSF](https://i.imgur.com/3xqAk8u.png)

### A Star

The exercise for this application was for me to implement the A star algorithm. 
