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
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 */
/*

Instinct demo starter applet.

Starts server-side demos of solving the Monkey and Bananas
problem by instinct evolution.

Reference:
T. E. Portegys, "Instinct Evolution in a Neural Network",
See www.itk.ilstu.edu/faculty/portegys/research.html#instinct
*/
package mona;

import java.applet.Applet;

import java.awt.*;
import java.awt.event.*;

import java.io.*;

import java.net.*;

import java.util.*;

import javax.swing.*;
import javax.swing.event.*;


public class InstinctDemoStarter extends JApplet implements ActionListener {
    // Default server address.
    static final String DEFAULT_SERVER_HOST = "localhost";
    static final int DEFAULT_SERVER_PORT = 7672;

    // Server.
    String serverHost;
    int serverPort;
    InetAddress serverAddress;
    String localHost;
    Socket socket;
    BufferedReader in;
    PrintWriter out;
    String error;

    // New demo button.
    JButton newDemo;

    // Applet information.
    public String getAppletInfo() {
        return ("Instinct Demo Starter, Copyright (c) 2005 Tom Portegys, All Rights Reserved (portegys@ilstu.edu) - Version 1.0");
    }

    // Initialize.
    public void init() {
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

        // Get local host name.
        localHost = null;

        try {
            localHost = InetAddress.getLocalHost().getHostAddress();
        } catch (Exception e) {
            error = "Cannot get of IP address of local host";
        }

        // Create the controls display.
        JPanel screen = (JPanel) getContentPane();
        screen.setLayout(new FlowLayout());
        screen.setBackground(Color.white);
        newDemo = new JButton("New Demo");
        newDemo.addActionListener(this);
        screen.add(newDemo);
    }

    // Start.
    public void start() {
        // Connect to demo server.
        if (error == null) {
            try {
                socket = new Socket(serverAddress, serverPort);
                in = new BufferedReader(new InputStreamReader(
                            socket.getInputStream()));
                out = new PrintWriter(new BufferedWriter(
                            new OutputStreamWriter(socket.getOutputStream())),
                        true);

                // Send this address.
                out.println("address " + localHost);

                // Send new command
                out.println("new");
                out.flush();
            } catch (Exception e) {
                error = "Cannot connect to server: " + serverHost + ":" +
                    serverPort;
            }
        } else {
            showStatus(error);
        }
    }

    // Stop.
    public synchronized void stop() {
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

    // Button listener.
    public void actionPerformed(ActionEvent evt) {
        if (error != null) {
            showStatus(error);

            return;
        }

        if (evt.getSource() == newDemo) {
            out.println("new");
            out.flush();
        }
    }
}
