# Comparing-Different-Pathfinding-Algorithms

## What is Path finding?

**Path finding** in applications is the research of finding the shortest and/or cheapest (optimal) route between 2 points, usually on graphs. To achieve this, many different algorithms are used, all having better performance in certain situations. These are heavily based on **Dijkstra's algorithm**, which is an algorithm Created by **Edsger W. Dijkstra** in 1956 to find the shortest path between 2 **nodes** on a graph. This algorithm gets used in, for example, modern GPS systems to find the shortest path to get to your destination. In this Research project, I used the framework provided by my college (**DAE Howest**) to compare 2 well known path finding algorithms, and 1 lesser known algorithm that is a variant on 1 of these 2, to find the most optimal one. These algorithms being:
* Breadth First Search (BFS)
* A Star (A*)
* Jump Point Search (JPS)

BFS and A* I had to implement during my classes Gameplay programming, so this research will mostly be focusing on the implementation of JPS.

## Color code for the images of the algorithms

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

The exercise for this application was for me to implement the A star algorithm. This algorithm is a **Best-first search** algorithm. This means that every time it goes over all possible paths, it will only extend the paths that are most likely to be the most optimal routes to the goal node. This way A* does not guarantee the shortest path, but makes more than up for this by being as fast as it is. This algorithm is fast and is very commonly used in video games, for example Age of Empires. 

![Image example of A*](https://i.imgur.com/aYjdJVt.png)

### Jump Point Search

As the focus of this project, I wanted to try and implement JPS myself. This is an improved version of A*, that works optimally in a cost uniform grid (all connection in a certain direction cost the same to follow). JPS works similarly to A* in that it will focus on the paths that are most likey to lead to the most optimal route to the goal node. The difference of JPS however, is that it will jump over nodes whenever it knows that it's fine to follow the path as far as possible. For example, if the start node first has to go 6 space to the right before turning, then JPS will jump from the first node, to the 6th node to instantly recalculate from that point where it will go next.

![Image example of JPS](https://i.imgur.com/4wKZCJB.png)

To be able to do this jumping, JPS prunes neighbors of the node it goes to to find out if the path to those neighbors are dominated by a different path that does not include the node that's being evaluated.
What this means for diagonal pathing: if there is a path, without going through the current node, coming from the parent of the current node to the neighbor that gets evaluated on pruning that is strictly shorter than the path if it was to go through the current node, then this neighbor gets pruned.
For horizontal pathing: if there is a path, without going through the current node, coming from the parent of the current node to the neighbor that gets evaluated on pruning that is shorter or equal in cost than the path if it was to go through the current node, then this neighbor gets pruned.

- Example of how this neighbor pruning works from the paper "Online Graph Pruning for Pathfinding on Grid Maps" by Daniel Harabor and Alban Grastien:

![Image example of pruning neighbors](https://i.imgur.com/WJIrUek.png)

JPS will keep looking for a point to jump to until one of 3 definitions has been met for the point to jump to:
1. The node is the goal node, so there is no point in continuing 
2. The node has atleast one neighbor that is forced. A neighbor can be forced if it is not a natural neighor of the current node, for example in the above image in B and D, respectively speaking node 3 and 1 are forced neighbors of x.
3. (only for diagonal direction) There is a neighbor of the current node that is a jump point from the goal node. 

- Example for the 3rd definition of a jump point (point y is a jump point of x because it has a neighbor that could be a jump point of the end node, which is z):

![Image example of jump point definition 3](https://i.imgur.com/ZV7Fi7y.png)

## Testing and comparing the different algorithms

To test the 3 different algorithms, I set up 3 different example situations where all 3 have to find the path from the same start to destination nodes. For every situation every algorithm was run 10 times and the average time was picked every time.

### situation 1:

![Image of the layout of situation 1](https://i.imgur.com/IpGmt3C.png)

#### BSF

![Image of BSF doing situation 1](https://i.imgur.com/TJ77maV.png)

The average time of BSF for situation 1 was 0.00819465 seconds. With the slowest time being 0.0091826 and the fastest time being 0.0077329 seconds.

#### A*

![Image of A* doing situation 1](https://i.imgur.com/Ow9ryOX.png)

The average time of A* for situation 1 was 0.06232561 seconds. With the slowest time being 0.0672769 and the fastest time being 0.0596939 seconds.

#### JPS

![Image of JPS doing situation 1](https://i.imgur.com/arHJx4i.png)

The average time of JPS for situation 1 was 0.0210867 seconds. With the slowest time being 0.021945 and the fastest time being 0.0206956 seconds.

### situation 2:

![Image of the layout of situation 2](https://i.imgur.com/L9kAbeS.png)

#### BSF

![Image of BSF doing situation 2](https://i.imgur.com/QwVdJ9A.png)

The average time of BSF for situation 2 was 0.00678009 seconds. With the slowest time being 0.0075059 and the fastest time being 0.0064703 seconds.

#### A*

![Image of A* doing situation 2](https://i.imgur.com/IRgSlWe.png)

The average time of A* for situation 2 was 0.06323752 seconds. With the slowest time being 0.0707086 and the fastest time being 0.0614413 seconds.

#### JPS

![Image of JPS doing situation 2](https://i.imgur.com/oXjHEnr.png)

The average time of JPS for situation 2 was 0.02381501 seconds. With the slowest time being 0.021945 and the fastest time being 0.0206956 seconds.

### situation 3:

Advanced layout with a lot of water to go around

![Image of the layout of situation 3](https://i.imgur.com/EERoWCi.png)

#### BSF

![Image of BSF doing situation 3](https://i.imgur.com/eQVShLj.png)

The average time of BSF for situation 3 was 0.00632271 seconds. With the slowest time being 0.0339036 and the fastest time being 0.0218067 seconds.

#### A*

![Image of A* doing situation 3](https://i.imgur.com/txBky59.png)

The average time of A* for situation 3 was 0.02757065 seconds. With the slowest time being 0.0265119 and the fastest time being 0.02845 seconds.

#### JPS

![Image of JPS doing situation 1](https://i.imgur.com/AXKa3zW.png)

JPS did not find a path to the end node in this situation.

## Conclusion

Due to the limitations of the framework, I was unable to code my JPS in such a way that it finds a path 100% of the time. When it does find a path however, it finds this path at more than double the speed of my A* application. The BSF that was coded with the professors of this class, is by far more efficiently coded though.

## References


[Online Graph Pruning for Pathfinding on Grid Maps by Daniel Harabor and Alban Grastien](http://grastien.net/ban/articles/hg-aaai11.pdf)

[Jump Point Search Example](https://gamedevelopment.tutsplus.com/tutorials/how-to-speed-up-a-pathfinding-with-the-jump-point-search-algorithm--gamedev-5818)

[Dijkstra's algorithm](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm#History)

[Pathfinding](https://en.wikipedia.org/wiki/Pathfinding)

[Breadth First Search Algorithm](https://en.wikipedia.org/wiki/Breadth-first_search)

[A* Search Algorithm](https://en.wikipedia.org/wiki/A*_search_algorithm)

[What is Jump Point Search (video)](https://www.youtube.com/watch?v=wNOoyZ45SmQ&t=251s&ab_channel=TECHDOSE)
