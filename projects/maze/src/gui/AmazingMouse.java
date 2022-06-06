/*
 * This software is provided under the terms of the GNU General
 * Public License as published by the Free Software Foundation.
 *
 * Copyright (c) 2005 Tom Portegys, All Rights Reserved.
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for NON-COMMERCIAL purposes and without
 * fee is hereby granted provided that this copyright notice
 * appears in all copies.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 */
/*

Maze-learning mouse.

A client for Mona maze context-learning task. Server runs Mona application.

The user can train a mouse to learn a path through a maze to obtain cheese.
The mouse is presented with an initial set of doors, only one of which leads
to the beginning of the maze. That door choice must be repeated at the end
of the maze in order to obtain the cheese. The intermediate task is navigating
a path through the maze.

Reference:
T. E. Portegys, "An Application of Context-Learning in a Goal-Seeking Neural Network",
The IASTED International Conference on Computational Intelligence (CI 2005).
See www.itk.ilstu.edu/faculty/portegys/research.html#context
*/
package mona;

import java.applet.Applet;
import java.applet.AudioClip;

import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;

import java.io.*;

import java.net.*;

import java.util.*;

import javax.swing.*;
import javax.swing.event.*;


public class AmazingMouse extends JApplet implements ActionListener,
    ItemListener, Runnable {
    // Room types.
    static final int START_ROOM = 0;
    static final int BEGIN_MAZE = 1;
    static final int END_MAZE = 2;
    static final int GOAL_ROOM = 3;
    static final int DEAD_ROOM = 4;
    static final int MAZE_ROOM = 5;

    // Special responses.
    static final int WAIT = 3;
    static final int HOP = 4;

    // Modes
    static final int TRAIN = 0;
    static final int RUN = 1;

    // Default server address.
    static final String DEFAULT_SERVER_HOST = "localhost";
    static final int DEFAULT_SERVER_PORT = 7671;

    // Step delay (ms).
    static final int STEP_DELAY = 1000;

    // Display update frequency (ms).
    static final int DISPLAY_DELAY = 50;

    // Mouse and cheese images.
    final static String MOUSE_IMAGE_FILE = "mouse.gif";
    final static String CHEESE_IMAGE_FILE = "cheese.jpg";

    // Mouse squeak sound.
    final static String SQUEAK_SOUND_FILE = "squeak.wav";

    // Server.
    String serverHost;
    int serverPort;
    InetAddress serverAddress;
    Socket socket;
    BufferedReader in;
    PrintWriter out;

    // Maze.
    Room[][] maze;
    Dimension roomSize;
    int beginMazeIndex;
    int endMazeIndex;
    int mouseX;
    int mouseY;

    // Display.
    Dimension screenSize;
    Canvas mazeCanvas;
    Dimension mazeSize;
    Graphics canvasGraphics;
    Image mazeImage;
    Graphics imageGraphics;
    String error;

    // Controls.
    JPanel controls;
    JCheckBox start;
    boolean trialActive;
    Choice mode;
    TextField response;
    JButton reset;

    // Font.
    Font font;
    FontMetrics fontMetrics;
    int fontAscent;
    int fontWidth;
    int fontHeight;

    // Images and sound.
    Image mouseImage;
    Image cheeseImage;
    AudioClip squeakSound;

    // Threads.
    private Thread displayThread;
    private Thread stepThread;
    Toolkit toolkit;

    // Applet information.
    public String getAppletInfo() {
        return ("Maze-learning Mouse, Copyright (c) 2005 Tom Portegys, All Rights Reserved (portegys@ilstu.edu) - Version 1.0");
    }

    // Initialize.
    public void init() {
        toolkit = Toolkit.getDefaultToolkit();

        // Identify server.
        error = null;
        socket = null;
        in = null;
        out = null;

        if ((serverHost = getParameter("ServerHost")) == null) {
            serverHost = DEFAULT_SERVER_HOST;
        }

        String s;

        if ((s = getParameter("ServerPort")) == null) {
            serverPort = DEFAULT_SERVER_PORT;
        } else {
            serverPort = Integer.parseInt(s);
        }

        serverAddress = null;

        try {
            serverAddress = InetAddress.getByName(serverHost);
        } catch (Exception e) {
            error = "Cannot get address for host: " + serverHost;
        }

        // Create the maze display.
        JPanel screen = (JPanel) getContentPane();
        screen.setLayout(new FlowLayout());
        screenSize = getSize();
        mazeSize = new Dimension(screenSize.width,
                (int) ((double) screenSize.height * .90));
        mazeCanvas = new Canvas();
        mazeCanvas.setSize(mazeSize.width, mazeSize.height);
        mazeCanvas.addMouseListener(new MazeMouseListener());
        screen.add(mazeCanvas);
        mazeImage = createImage(mazeSize.width, mazeSize.height);

        // Get font.
        font = new Font("Helvetica", Font.BOLD, 12);

        // Add controls.
        controls = new JPanel();
        controls.setLayout(new FlowLayout());
        start = new JCheckBox("Start Trial");
        start.addItemListener(this);
        start.setToolTipText("Go mouse!");
        trialActive = false;
        controls.add(start);
        controls.add(new JLabel("Mode:"));
        mode = new Choice();
        mode.add("Train");
        mode.add("Run");
        controls.add(mode);
        controls.add(new JLabel("Mouse Response:"));
        response = new TextField(10);
        response.setEditable(false);
        controls.add(response);
        reset = new JButton("Reset");
        reset.addActionListener(this);
        controls.add(reset);
        screen.add(controls);

        // No threads yet.
        displayThread = stepThread = null;

        // Build maze.
        int x;
        int y;
        Room[] doors;
        roomSize = new Dimension(mazeSize.width / 15, mazeSize.height / 11);
        maze = new Room[7][];
        maze[0] = new Room[3];
        x = roomSize.width;
        y = (mazeSize.height / 2) - (roomSize.height / 2);
        maze[0][1] = new Room(x, y, START_ROOM, 0, 1);
        maze[0][0] = new Room(x, y - (roomSize.height * 2), START_ROOM, 0, 0);
        maze[0][2] = new Room(x, y + (roomSize.height * 2), START_ROOM, 0, 2);
        beginMazeIndex = 1;
        maze[1] = new Room[1];
        x += (roomSize.width * 2);
        y = (mazeSize.height / 2) - (roomSize.height / 2);
        maze[1][0] = new Room(x, y, BEGIN_MAZE, 1, 0);
        doors = new Room[3];
        doors[0] = null;
        doors[1] = null;
        doors[2] = maze[1][0];
        maze[0][0].setDoors(doors);
        doors = new Room[3];
        doors[0] = null;
        doors[1] = maze[1][0];
        doors[2] = null;
        maze[0][1].setDoors(doors);
        doors = new Room[3];
        doors[0] = maze[1][0];
        doors[1] = null;
        doors[2] = null;
        maze[0][2].setDoors(doors);
        maze[2] = new Room[3];
        x += (roomSize.width * 2);
        y = (mazeSize.height / 2) - (roomSize.height / 2);
        maze[2][1] = new Room(x, y, MAZE_ROOM, 2, 1);
        maze[2][0] = new Room(x, y - (roomSize.height * 2), MAZE_ROOM, 2, 0);
        maze[2][2] = new Room(x, y + (roomSize.height * 2), MAZE_ROOM, 2, 2);
        doors = new Room[3];
        doors[0] = maze[2][0];
        doors[1] = maze[2][1];
        doors[2] = maze[2][2];
        maze[1][0].setDoors(doors);
        maze[3] = new Room[5];
        x += (roomSize.width * 2);
        y = (mazeSize.height / 2) - (roomSize.height / 2);
        maze[3][2] = new Room(x, y, MAZE_ROOM, 3, 2);
        maze[3][1] = new Room(x, y - (roomSize.height * 2), MAZE_ROOM, 3, 1);
        maze[3][0] = new Room(x, y - (roomSize.height * 4), MAZE_ROOM, 3, 0);
        maze[3][3] = new Room(x, y + (roomSize.height * 2), MAZE_ROOM, 3, 3);
        maze[3][4] = new Room(x, y + (roomSize.height * 4), MAZE_ROOM, 3, 4);
        doors = new Room[3];
        doors[0] = maze[3][0];
        doors[1] = maze[3][1];
        doors[2] = maze[3][2];
        maze[2][0].setDoors(doors);
        doors = new Room[3];
        doors[0] = maze[3][1];
        doors[1] = maze[3][2];
        doors[2] = maze[3][3];
        maze[2][1].setDoors(doors);
        doors = new Room[3];
        doors[0] = maze[3][2];
        doors[1] = maze[3][3];
        doors[2] = maze[3][4];
        maze[2][2].setDoors(doors);
        maze[4] = new Room[3];
        x += (roomSize.width * 2);
        y = (mazeSize.height / 2) - (roomSize.height / 2);
        maze[4][1] = new Room(x, y, MAZE_ROOM, 4, 1);
        maze[4][0] = new Room(x, y - (roomSize.height * 2), MAZE_ROOM, 4, 0);
        maze[4][2] = new Room(x, y + (roomSize.height * 2), MAZE_ROOM, 4, 2);
        doors = new Room[3];
        doors[0] = null;
        doors[1] = null;
        doors[2] = maze[4][0];
        maze[3][0].setDoors(doors);
        doors = new Room[3];
        doors[0] = null;
        doors[1] = maze[4][0];
        doors[2] = maze[4][1];
        maze[3][1].setDoors(doors);
        doors = new Room[3];
        doors[0] = maze[4][0];
        doors[1] = maze[4][1];
        doors[2] = maze[4][2];
        maze[3][2].setDoors(doors);
        doors = new Room[3];
        doors[0] = maze[4][1];
        doors[1] = maze[4][2];
        doors[2] = null;
        maze[3][3].setDoors(doors);
        doors = new Room[3];
        doors[0] = maze[4][2];
        doors[1] = null;
        doors[2] = null;
        maze[3][4].setDoors(doors);
        endMazeIndex = 5;
        maze[5] = new Room[1];
        x += (roomSize.width * 2);
        y = (mazeSize.height / 2) - (roomSize.height / 2);
        maze[5][0] = new Room(x, y, END_MAZE, 5, 0);
        doors = new Room[3];
        doors[0] = null;
        doors[1] = null;
        doors[2] = maze[5][0];
        maze[4][0].setDoors(doors);
        doors = new Room[3];
        doors[0] = null;
        doors[1] = maze[5][0];
        doors[2] = null;
        maze[4][1].setDoors(doors);
        doors = new Room[3];
        doors[0] = maze[5][0];
        doors[1] = null;
        doors[2] = null;
        maze[4][2].setDoors(doors);
        maze[6] = new Room[3];
        x += (roomSize.width * 2);
        y = (mazeSize.height / 2) - (roomSize.height / 2);
        maze[6][1] = new Room(x, y, GOAL_ROOM, 6, 1);
        maze[6][0] = new Room(x, y - (roomSize.height * 2), GOAL_ROOM, 6, 0);
        maze[6][2] = new Room(x, y + (roomSize.height * 2), GOAL_ROOM, 6, 2);
        doors = new Room[3];
        doors[0] = maze[6][0];
        doors[1] = maze[6][1];
        doors[2] = maze[6][2];
        maze[5][0].setDoors(doors);
        doors = new Room[3];
        doors[0] = null;
        doors[1] = null;
        doors[2] = null;
        maze[6][0].setDoors(doors);
        maze[6][1].setDoors(doors);
        maze[6][2].setDoors(doors);
        mouseX = mouseY = 0;
        maze[0][0].hasMouse = true;
        maze[0][0].selected = true;
        maze[endMazeIndex + 1][2].hasCheese = true;
        maze[endMazeIndex + 1][2].selected = true;

        // Get mouse and cheese images.
        Image mouse = null;
        mouseImage = null;

        Image cheese = null;
        cheeseImage = null;

        int w1;
        int h1;
        int w2;
        int h2;

        URL imgURL = AmazingMouse.class.getResource(MOUSE_IMAGE_FILE);

        if (imgURL != null) {
            mouse = new ImageIcon(imgURL).getImage();
        }

        if (mouse == null) {
            showStatus("Cannot get image: " + MOUSE_IMAGE_FILE);
        }

        imgURL = AmazingMouse.class.getResource(CHEESE_IMAGE_FILE);

        if (imgURL != null) {
            cheese = new ImageIcon(imgURL).getImage();
        }

        if (cheese == null) {
            showStatus("Cannot get image: " + CHEESE_IMAGE_FILE);
        }

        if (mouse != null) {
            w1 = mouse.getWidth(this);
            h1 = mouse.getHeight(this);

            if ((w1 <= 0) || (h1 <= 0)) {
                showStatus("Invalid image: " + MOUSE_IMAGE_FILE);
            } else {
                w2 = roomSize.width / 2;
                h2 = roomSize.height / 2;
                mouseImage = createImage(w2, h2);

                Graphics graphics = mouseImage.getGraphics();
                graphics.drawImage(mouse, 0, 0, w2 - 1, h2 - 1, 0, 0, w1 - 1,
                    h1 - 1, Color.white, this);
            }
        }

        if (cheese != null) {
            w1 = cheese.getWidth(this);
            h1 = cheese.getHeight(this);

            if ((w1 <= 0) || (h1 <= 0)) {
                showStatus("Invalid image: " + CHEESE_IMAGE_FILE);
            } else {
                w2 = roomSize.width / 2;
                h2 = roomSize.height / 2;
                cheeseImage = createImage(w2, h2);

                Graphics graphics = cheeseImage.getGraphics();
                graphics.drawImage(cheese, 0, 0, w2 - 1, h2 - 1, 0, 0, w1 - 1,
                    h1 - 1, Color.white, this);
            }
        }

        // Load mouse squeak sound.
        squeakSound = null;

        URL url = AmazingMouse.class.getResource(SQUEAK_SOUND_FILE);

        if (url != null) {
            squeakSound = Applet.newAudioClip(url);
        }

        if (squeakSound == null) {
            showStatus("Cannot load sound file: " + SQUEAK_SOUND_FILE);
        } else {
            // Playing and stopping insures sound completely loaded.
            squeakSound.play();
            squeakSound.stop();
        }
    }

    // Start.
    public void start() {
        // Connect to Mona server, and initialize parameters.
        if (error == null) {
            try {
                socket = new Socket(serverAddress, serverPort);
                in = new BufferedReader(new InputStreamReader(
                            socket.getInputStream()));
                out = new PrintWriter(new BufferedWriter(
                            new OutputStreamWriter(socket.getOutputStream())),
                        true);

                // #sensors=4, max response=HOP, #needs=1
                out.println("p 5 " + HOP + " 1");

                // Level 1 mediator event timer=10
                out.println("t 1 0 10");
                out.flush();

                // Get MAX_MEDIATOR_LEVEL string.
                in.readLine();
            } catch (Exception e) {
                error = "Cannot connect to server: " + serverHost + ":" +
                    serverPort;
            }
        }

        if (displayThread == null) {
            displayThread = new Thread(this);
            displayThread.setPriority(Thread.MIN_PRIORITY);
            displayThread.start();
        }

        if ((stepThread == null) && (error == null)) {
            stepThread = new Thread(this);
            stepThread.setPriority(Thread.MIN_PRIORITY);
            stepThread.start();
        }
    }

    // Stop.
    public synchronized void stop() {
        if (displayThread != null) {
            displayThread.interrupt();

            try {
                displayThread.join();
            } catch (InterruptedException e) {
            }

            displayThread = null;
        }

        if (stepThread != null) {
            stepThread.interrupt();

            try {
                stepThread.join();
            } catch (InterruptedException e) {
            }

            stepThread = null;
        }

        if (socket != null) {
            out.println("quit");
            out.flush();

            try {
                socket.close();
            } catch (Exception e) {
            }

            socket = null;
        }
    }

    // Run.
    public void run() {
        // display loop.
        while ((Thread.currentThread() == displayThread) &&
                !displayThread.isInterrupted()) {
            display();

            try {
                Thread.sleep(DISPLAY_DELAY);
            } catch (InterruptedException e) {
                break;
            }
        }

        // Step loop.
        while ((Thread.currentThread() == stepThread) &&
                !stepThread.isInterrupted()) {
            step();

            try {
                Thread.sleep(STEP_DELAY);
            } catch (InterruptedException e) {
                break;
            }
        }
    }

    // Display.
    synchronized void display() {
        // Get graphics.
        if (canvasGraphics == null) {
            if ((canvasGraphics = mazeCanvas.getGraphics()) == null) {
                return;
            }

            imageGraphics = mazeImage.getGraphics();

            canvasGraphics.setFont(font);
            imageGraphics.setFont(font);
            fontMetrics = canvasGraphics.getFontMetrics();
            fontAscent = fontMetrics.getMaxAscent();
            fontWidth = fontMetrics.getMaxAdvance();
            fontHeight = fontMetrics.getHeight();
        }

        // Clear display.
        imageGraphics.setColor(Color.white);
        imageGraphics.fillRect(0, 0, mazeSize.width, mazeSize.height);

        // Draw maze.
        for (int i = 0; i < maze.length; i++) {
            for (int j = 0; j < maze[i].length; j++) {
                maze[i][j].draw(imageGraphics);
            }
        }

        // Label maze.
        int h = fontHeight / 2;
        imageGraphics.drawString("Start", maze[0][0].x + 4, maze[0][0].y - h);
        imageGraphics.drawString("Entry", maze[beginMazeIndex][0].x + 4,
            maze[beginMazeIndex][0].y - h);
        imageGraphics.drawString("Maze", maze[beginMazeIndex + 1][0].x + 6,
            maze[beginMazeIndex + 1][0].y - h);
        imageGraphics.drawString("Maze", maze[beginMazeIndex + 2][0].x + 6,
            maze[beginMazeIndex + 2][0].y - h);
        imageGraphics.drawString("Maze", maze[beginMazeIndex + 3][0].x + 6,
            maze[beginMazeIndex + 3][0].y - h);
        imageGraphics.drawString("Exit", maze[endMazeIndex][0].x + 6,
            maze[endMazeIndex][0].y - h);
        imageGraphics.drawString("Goal", maze[endMazeIndex + 1][0].x + 6,
            maze[endMazeIndex + 1][0].y - h);

        // Show error message?
        if (error != null) {
            imageGraphics.setColor(Color.white);

            int w = fontMetrics.stringWidth(error);
            imageGraphics.fillRect(((mazeSize.width - w) / 2) - 2,
                (mazeSize.height / 2) - (fontAscent + 2), w + 4, fontHeight +
                4);
            imageGraphics.setColor(Color.black);
            imageGraphics.drawString(error, (mazeSize.width - w) / 2,
                mazeSize.height / 2);
        }

        // Refresh display.
        canvasGraphics.drawImage(mazeImage, 0, 0, this);
    }

    // Step.
    void step() {
        if (!start.isSelected() || (mouseX == -1) || !trialActive ||
                (error != null)) {
            return;
        }

        // If training, set correct response.
        int cx = -1;
        int cy = -1;
        int r = -1;
        int i;
        int j;

        if (mode.getSelectedIndex() == TRAIN) {
            for (i = mouseX + 1; (i < maze.length) && (cx == -1); i++) {
                for (j = 0; (j < maze[i].length) && (cx == -1); j++) {
                    if (maze[i][j].selected) {
                        cx = i;
                        cy = j;
                    }
                }
            }

            if (cx == (mouseX + 1)) {
                for (i = 0;
                        (i < maze[mouseX][mouseY].doors.length) && (r == -1);
                        i++) {
                    if ((maze[mouseX][mouseY].doors[i] != null) &&
                            (maze[mouseX][mouseY].doors[i].cx == cx) &&
                            (maze[mouseX][mouseY].doors[i].cy == cy)) {
                        r = i;

                        break;
                    }
                }
            } else {
                if (cx == -1) {
                    cx = mouseX;
                    cy = mouseY;
                    r = WAIT;
                }
            }

            if (r == -1) {
                r = HOP;
            }
        }

        out.println("r " + r);
        out.flush();

        // If found cheese, decrement need.
        if (maze[mouseX][mouseY].hasCheese) {
            if (squeakSound != null) {
                squeakSound.play();
            }

            out.println("n 0 5.0");
            out.flush();
        }

        // Initiate sensory/response cycle.
        out.print("c ");

        for (i = 0; i < maze[mouseX][mouseY].doors.length; i++) {
            if (maze[mouseX][mouseY].doors[i] == null) {
                out.print("0 ");
            } else {
                out.print("1 ");
            }
        }

        out.print(maze[mouseX][mouseY].type + " ");

        if (maze[mouseX][mouseY].hasCheese) {
            out.println("1");

            // Eat the cheese.
            maze[mouseX][mouseY].hasCheese = false;
        } else {
            out.println("0");
        }

        out.flush();

        // Move mouse based on response.
        try {
            r = Integer.parseInt(in.readLine());
        } catch (Exception e) {
            error = "Error getting response from server";

            return;
        }

        if (r < WAIT) {
            response.setText("Door " + r);
        } else if (r == WAIT) {
            response.setText("Wait");
        } else {
            response.setText("Hop");
        }

        if (mode.getSelectedIndex() == TRAIN) {
            maze[mouseX][mouseY].hasMouse = false;
            mouseX = cx;
            mouseY = cy;
            maze[mouseX][mouseY].hasMouse = true;

            if (r == WAIT) {
                trialActive = false;
            }
        } else {
            // Is response allowed?
            if ((r < WAIT) && (maze[mouseX][mouseY].doors[r] != null) &&
                    maze[maze[mouseX][mouseY].doors[r].cx][maze[mouseX][mouseY].doors[r].cy].selected) {
                maze[mouseX][mouseY].hasMouse = false;
                cx = maze[mouseX][mouseY].doors[r].cx;
                cy = maze[mouseX][mouseY].doors[r].cy;
                mouseX = cx;
                mouseY = cy;
                maze[mouseX][mouseY].hasMouse = true;
            } else {
                // Check for available move.
                for (i = 0; i < maze[mouseX][mouseY].doors.length; i++) {
                    if ((maze[mouseX][mouseY].doors[i] != null) &&
                            maze[maze[mouseX][mouseY].doors[i].cx][maze[mouseX][mouseY].doors[i].cy].selected) {
                        break;
                    }
                }

                if (i == maze[mouseX][mouseY].doors.length) {
                    trialActive = false;
                }
            }
        }
    }

    // Button listener.
    public void actionPerformed(ActionEvent evt) {
        if (error != null) {
            return;
        }

        if (evt.getSource() == reset) {
            // Erase mouse memory.
            out.println("e stm");
            out.println("e ltm");
            out.flush();

            // Clear maze.
            for (int i = 0; i < maze.length; i++) {
                for (int j = 0; j < maze[i].length; j++) {
                    maze[i][j].selected = false;
                    maze[i][j].hasMouse = false;
                    maze[i][j].hasCheese = false;
                }
            }

            mouseX = mouseY = 0;
            maze[0][0].selected = true;
            maze[0][0].hasMouse = true;
            maze[endMazeIndex + 1][2].hasCheese = true;
            start.setSelected(false);
            start.setText("Start Trial");
            response.setText("");
        }
    }

    // CheckBox listener.
    public void itemStateChanged(ItemEvent evt) {
        if (error != null) {
            return;
        }

        if (evt.getSource() == start) {
            if (start.isSelected()) {
                start.setText("Stop Trial");
                trialActive = true;

                // Initialize need and goal state.
                out.println("n 0 10.0");
                out.println("g 0 0 0 " + GOAL_ROOM + " 1 5.0");

                // Clear working memory.
                out.println("e stm");
                out.flush();
            } else {
                start.setText("Start Trial");
                trialActive = false;
                response.setText("");

                // Place cheese in selected goal room.
                for (int i = 0; i < maze[endMazeIndex + 1].length; i++) {
                    if (maze[endMazeIndex + 1][i].selected) {
                        maze[endMazeIndex + 1][i].hasCheese = true;

                        break;
                    }
                }

                // Move mouse back to leftmost selected room.
                if (mouseX != -1) {
                    for (int i = 0; i < maze.length; i++) {
                        for (int j = 0; j < maze[i].length; j++) {
                            if (maze[i][j].selected) {
                                maze[mouseX][mouseY].hasMouse = false;
                                maze[i][j].hasMouse = true;
                                mouseX = i;
                                mouseY = j;

                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    // Maze room.
    class Room extends Rectangle {
        int type;
        int cx;
        int cy;
        boolean selected;
        boolean hasMouse;
        boolean hasCheese;
        Room[] doors;

        Room(int x, int y, int type, int cx, int cy) {
            super(x, y, roomSize.width, roomSize.height);
            this.type = type;
            this.cx = cx;
            this.cy = cy;
            selected = hasMouse = hasCheese = false;
        }

        // Set room doors to other rooms.
        void setDoors(Room[] doors) {
            this.doors = new Room[doors.length];

            for (int i = 0; i < doors.length; i++) {
                this.doors[i] = doors[i];
            }
        }

        // Draw the room.
        void draw(Graphics g) {
            Room to;

            g.setColor(Color.black);

            if (selected) {
                g.fillRect(x, y, width, height);
                g.setColor(Color.white);
                g.fillRect(x + 4, y + 4, width - 8, height - 8);
                g.setColor(Color.black);
            } else {
                g.drawRect(x, y, width, height);
            }

            if (hasCheese && (cheeseImage != null)) {
                g.drawImage(cheeseImage, x + (width / 4), y + (height / 4),
                    mazeCanvas);
            }

            if (hasMouse && (mouseImage != null)) {
                g.drawImage(mouseImage, x + (width / 4), y + (height / 4),
                    mazeCanvas);
            }

            for (int i = 0; i < doors.length; i++) {
                if ((to = doors[i]) != null) {
                    g.drawLine(x + width, y + (height / 2), to.x,
                        to.y + (to.height / 2));
                }
            }
        }
    }

    // Mouse press.
    class MazeMouseListener extends MouseAdapter {
        public void mousePressed(MouseEvent evt) {
            if (error != null) {
                return;
            }

            // Select/deselect room.
            boolean found = false;
            int cx = 0;
            int cy = 0;

            for (int i = 0; (i < maze.length) && !found; i++) {
                for (int j = 0; (j < maze[i].length) && !found; j++) {
                    if (maze[i][j].contains(evt.getX(), evt.getY())) {
                        found = true;
                        cx = i;
                        cy = j;
                    }
                }
            }

            if (!found) {
                return;
            }

            if (start.isSelected()) {
                toolkit.beep();

                return;
            }

            maze[cx][cy].selected = !maze[cx][cy].selected;

            // Move mouse/cheese if necessary.
            if (maze[cx][cy].selected) {
                for (int i = 0; i < maze[cx].length; i++) {
                    if ((i != cy) && maze[cx][i].selected) {
                        maze[cx][i].selected = false;

                        if (maze[cx][i].hasMouse) {
                            mouseX = mouseY = -1;
                        }

                        maze[cx][i].hasMouse = false;
                        maze[cx][i].hasCheese = false;
                    }
                }

                if ((mouseX != -1) && (mouseX > cx)) {
                    maze[mouseX][mouseY].hasMouse = false;
                    mouseX = mouseY = -1;
                }

                if (mouseX == -1) {
                    maze[cx][cy].hasMouse = true;
                    mouseX = cx;
                    mouseY = cy;
                }

                if (cx == (endMazeIndex + 1)) {
                    maze[cx][cy].hasCheese = true;
                }
            } else {
                if (maze[cx][cy].hasMouse) {
                    mouseX = mouseY = -1;

                    for (int i = cx + 1; (i < maze.length) && (mouseX == -1);
                            i++) {
                        for (int j = 0; (j < maze[i].length) && (mouseX == -1);
                                j++) {
                            if (maze[i][j].selected) {
                                maze[i][j].hasMouse = true;
                                mouseX = i;
                                mouseY = j;
                            }
                        }
                    }
                }

                maze[cx][cy].hasMouse = false;
                maze[cx][cy].hasCheese = false;
            }
        }
    }
}
