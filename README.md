# Multithreaded Order Matching Engine with Real-Time Frontend

## Overview
This project is a **multithreaded order matching engine** implemented in C++ with a full end-to-end architecture that allows users to submit, process, and visualize orders in real-time. It demonstrates key concepts in **low-latency systems, distributed messaging, and frontend visualization**.

The system is designed to simulate a simplified trading environment with a live order book that users can interact with via a modern React frontend.

## Architecture
The architecture consists of:

1. **C++ Order Matching Engine**  
   - Multithreaded TCP server for handling incoming orders concurrently.  
   - Maintains the order book and executes trades according to price-time priority.  

2. **Apache Kafka**  
   - Used as the messaging backbone to stream updates of the order book state.  
   - Ensures decoupled and scalable communication between backend and frontend services.  

3. **Node.js Gateway**  
   - Consumes order book events from Kafka.  
   - Exposes WebSocket endpoints to the frontend.  
   - Handles user order submissions from the React frontend.  

4. **React Frontend**  
   - Allows users to submit new orders.  
   - Displays a real-time visualization of the **top 2 levels of the order book (Quote II Book)**.  

## Features
- High-performance multithreaded order processing.  
- Real-time updates of order book state using Kafka.  
- Full-stack integration with a Node.js API gateway.  
- Interactive React-based visualization of live market data.  
- Support for submitting buy and sell orders directly from the frontend.

