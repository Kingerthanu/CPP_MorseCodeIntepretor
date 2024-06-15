# CPP_MorseCodeIntepretor

  A Java project created in 2021 where in a javafx GUI a user is able to use their cursor to draw walls. The walls cycle through the color spectrum and are used in creating a labrinthe in order for a given entry point to go to a given end point in the labrinthe. The user can use a single left-click in order to place the starting point and right click to add the end point. 

<img src="https://github.com/Kingerthanu/Java_Astar_PathfindingFreeDraw/assets/76754592/702d84e4-cacc-4a42-939d-4c26bd716ff5" alt="Cornstarch <3" width="85" height="89">

----------------------------------------------

<img src="https://github.com/Kingerthanu/Java_Astar_PathfindingFreeDraw/assets/76754592/f436ac54-c4e1-4865-b40b-cb1514fc3638" alt="Cornstarch <3" width="65" height="49"><img src="https://github.com/Kingerthanu/Java_Astar_PathfindingFreeDraw/assets/76754592/f436ac54-c4e1-4865-b40b-cb1514fc3638" alt="Cornstarch <3" width="65" height="49"><img src="https://github.com/Kingerthanu/Java_Astar_PathfindingFreeDraw/assets/76754592/f436ac54-c4e1-4865-b40b-cb1514fc3638" alt="Cornstarch <3" width="65" height="49"><img src="https://github.com/Kingerthanu/Java_Astar_PathfindingFreeDraw/assets/76754592/f436ac54-c4e1-4865-b40b-cb1514fc3638" alt="Cornstarch <3" width="65" height="49">


**The Breakdown:**

  This Java Project Was Made With JavaFX For The Window.

  The Screen Is Divided Into A Equally-Sided 2D-Array Of Blocks In Which Will Represent Our Labrinths Playspace. 

  A Block Is A Defined ClassType In Which Contains Its xy-Coordinates As Well As Its Type ID. It Also Encapsulates Its Behaivor When Detecting Differing Types Of Clicks On Itself By The Users Cursor (I.E. Changing Its Block TypeID From Wall -> Start, etc.).
  
  The Program Works Mainly Upon The Users Cursor. And Wishes To Use An A* Algorithm To Pathfind Throughout A User-Defined Labrinth; Showing The Resulting Path On The Screen.

  The User Can Specify The Layout Of The Labrinth By Using Their Cursor As Controls Consists Of:

 &nbsp; - Left-Click + No Drag: Initilize The Starting Block.

 &nbsp; - Left-Click + Yes Drag: Initialize A Wall Block On All Blocks Dragged Over

 &nbsp; - Right-Click: Initilize The Ending Block.

 &nbsp; - Enter: Commit A A* Traversal From Starting -> Ending Block.

  
  When the user clicks ender, a A* algorithmn will traverse the given 2D matrix representation of this labrinthe in order to get to the exit. The quickest path will be traced in a purple line connecting the two points and the console will print out the step order to arrive there.

  Also Seperately On The Main Thread We Have A Random Gradient Going On With THe Drawn Wall Nodes Just As Visual Stimulation. It Was Fun Implementing For Me And Taught Me Some Things About State Changes When Drawing Something As Static.

<img src="https://github.com/Kingerthanu/Java_Astar_PathfindingFreeDraw/assets/76754592/da0b65f3-1131-4ab0-9f54-76625658191a" alt="Cornstarch <3" width="65" height="49"><img src="https://github.com/Kingerthanu/Java_Astar_PathfindingFreeDraw/assets/76754592/da0b65f3-1131-4ab0-9f54-76625658191a" alt="Cornstarch <3" width="65" height="49"><img src="https://github.com/Kingerthanu/Java_Astar_PathfindingFreeDraw/assets/76754592/da0b65f3-1131-4ab0-9f54-76625658191a" alt="Cornstarch <3" width="65" height="49"><img src="https://github.com/Kingerthanu/Java_Astar_PathfindingFreeDraw/assets/76754592/da0b65f3-1131-4ab0-9f54-76625658191a" alt="Cornstarch <3" width="65" height="49">

----------------------------------------------

<img src="https://github.com/Kingerthanu/Java_Astar_PathfindingFreeDraw/assets/76754592/98a9e049-55fe-4cf6-9171-971da1d1b014" alt="Cornstarch <3" width="65" height="49"><img src="https://github.com/Kingerthanu/Java_Astar_PathfindingFreeDraw/assets/76754592/98a9e049-55fe-4cf6-9171-971da1d1b014" alt="Cornstarch <3" width="65" height="49"><img src="https://github.com/Kingerthanu/Java_Astar_PathfindingFreeDraw/assets/76754592/98a9e049-55fe-4cf6-9171-971da1d1b014" alt="Cornstarch <3" width="65" height="49"><img src="https://github.com/Kingerthanu/Java_Astar_PathfindingFreeDraw/assets/76754592/98a9e049-55fe-4cf6-9171-971da1d1b014" alt="Cornstarch <3" width="65" height="49">

