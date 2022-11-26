# Concepts-of-Multi-Threading-and-Networking

## Multi-Threading

### Question-1

- Implementation has been done using threads in pthread library.
- Compilation requires -lpthread flag : gcc -lpthread q1.c
- The test case input is as mentioned in the question

- Implementation uses semaphore for the number of washing machines free, which is initialized to the number of washing machines initially.
- At every event, the time instant from the start of execution is printed.
- Color coding is used for specific events.
- No mutex locks are used as the semaphore automatically locks and unlocks based on availability of the value.

### Question-2

- Implementation has been done using threads in pthread library.
- Compilation requires -pthread flag : gcc -pthread q2.c
- The test case input is as mentioned in the question

- Implementation uses semaphore for the number of ovens free, which is initialized to the number of ovens initially.
- Mutex locks have been user for all other resources.
- Each customer and each chef is a thread. 
- At every event, the time instant from the start of execution is printed.
- Color coding is used for specific events.

Algorithm:
- Each customer aand chef are assigned to a thread.
- Ovens is used as semaphore
- v_num_chefs, v_num_chefs_left, v_num_orders, v_ingredients, v_order are all mutex locks, which imply mutexes for number of chefs in restaurant, number of 
chefs left to come, number of orders processing, number of ingredients and number of waiting orders. These locks are acquired and released for changing the 
corresponding variables
- logic of the Implementation goes sequentially from customer thread creating an order, chefs being allocated the order, chefs processing each pizza in the 
order after checking ingredients availability, waiting for oven semaphore to get free, baking the pizza, and finally, making the customer wait till he 
reaches the pickup zone to pick up the completed/partially completed order.

1. In the situation when the pizza storage is limited and sent to secondary storage, we can keep a counter and once the counter goes above the maximum, we route
 those specific orders and customers to the secondary storage.
2. As soon as the order is received, check all pizzas have enough ingredients and reject the order then and there without assigning to chef.
3. The drive-thru will now have future information, if there is a possibility of getting ingredients, and after getting them, if the orders can be completed, 
then the order is accepted even if there are no sufficient ingredients currently available.

## Networking

Implementation has been done using threads in the pthread library.
Compilation must be done together : gcc client.cpp server.cpp -lpthread

Working of the server code:
- Firstly, the direct connections in the network are given as inputted to server.cpp
- These inputs given constitute a graph, and a weighted, undirected graph is constructed
- Once the graph is constructed, we perform dijkstra algorithm over it to identify the shortest path and smallest delay between node 0 and all other nodes

Working of the client code:
- This code is a wrapper code, which acts like a shell for the server code to run
- The commands supported are pt and send

pt:
usage - pt
result - prints the routing table as given in the pdf

send:
usage - send 3 Hello!
result - sends the message 'Hello!' to node 3 starting from node 0, sends the packet from node 0 to node 3 using the shortest path calculated earlier,
and each node is a thread. Message passing between threads is implemented. For simulation purpose, the message is assumed to take one second to go from one 
server node to another, and the current node, destination node, forwareded node and the message are printed at every transmission
constraint - the message shouldn't have any spaces

Some additional points:
- Make sure to give a space ' ' after the last digit in the graph specification

Input format:
```
4 5
0 1 15
0 2 50
1 2 15
2 3 20
0 3 40 

pt

send 2 Hello!
```
If servers fail, all edges to that server are completely removed, and another routing path is found, which would require re-computation of the routing table 
and finally, there might also be situations where the graph is no more connected and in which case, some nodes may not be reachable.
