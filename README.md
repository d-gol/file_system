# File System

# What is it about?

This project is an implementation of a file system for reading and writing files to a hard drive. Hard drive was given as a C++ library with only two API calls: readCluster() and writeCluster(). The goal is to implement a file system that provides structured access to all files on the hard drive and is able to read/write entire files to memory.

Files on the hard drive are divided in clusters with fixed size. FAT allocation is used to support the work with clusters. Only one cluster can be cached in the memory at any given time.

The system allows multithreading.

# How is it implemented?

This project is implemented using C++. FAT allocation for clusters is implemented as an array where each index contains the next element in the file list. If the value is 0, this is the end of file. Data is operated cluster by cluster, for both read and write operations. Multi-threading is implemented via buffering, with FIFO queue. The project is tested with JPG images and 3 simultaneous threads.
