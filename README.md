# Hot Potatoes game simulation

The project simulates the hot potato game over multiple nodes in a network of computer.
It has its own protocol to commul=icate between nodes over TCP sockets. 

# How the game works
The *master* node passes the *potato* to one of the player at random. Each player can
pass the potato to its left or  right neighbour. Once the maximum number of passes are
exhausted, the potato is returned to master and master prints the complete passing history 
of the potato. This history is appended by each node that recieves the potato during the game.
