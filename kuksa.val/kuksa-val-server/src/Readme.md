
# Concept

```

    +----------------------+                    
    | IVssCommandProcessor |
    +----------------------+                    
        ^                          
        -                          
        |                          
        |                          
        |                          
    +---------------------+      
    | VssCommandProcessor |                                                                  
    +---------------------+                                                                  
    ^   |                        +---------+                                              
    .   |                        | ILogger |                                              
    .   |                        +---------+                                              
    .   |                            ^                                                    
    .   |                            -                                                    
    .   |                            |                                                    
    .   |                        +-------------+                                           
    .   +----------------------->| BasicLogger |<-----....(every class)
    .   |                        +-------------+                                          
    .   |                                                                                 
    .   |                        +----------------+              +----------------+         
    .   |                        | IAuthenticator |              | IAccessChecker |      
    .   |                        +----------------+              +----------------+      
    .   |                            ^                               ^                      
    .   |                            -                               -                   
    .   |                            |                               |                   
    .   |                            |                               |                   
    .   |                        +---------------+               +---------------+       
    .   +----------------------->| Authenticator |<----------+---| AccessChecker |
    .   |                        +---------------+           |   +---------------+       
    .   |                                                    |           ^               
    .   |                        +--------------+            |           |                
    .   |                        | IVssDatabase |            |           |                
    .   |                        +--------------+            |           |                
    .   |                            ^                       |           |                
    .   |                            -   +----------+        |           |                
    .   |                            +-->| Recorder |        |           |                
    .   |                            |   +----------+        |           |                
    .   |                        +-------------+             |           |                
    .   +----------------------->| VssDatabase |<------------+           |
    .   |                        +-------------+             |           |
    .   |                                                    |           |
    .   |                        +----------------------+    |           |               
    .   |                        | ISubscriptionHandler |    |           |               
    .   |                        +----------------------+    |           |               
    .   |                            ^                       |           |               
    .   |                            -                       |           |               
    .   |                            |                       |           |               
    .   |                            |                       |           |               
    .   |                        +---------------------+     |           |               
    .   +----------------------->| SubscriptionHandler |-----+-----------+               
    .                            +---------------------+
    .                                |                                                   
    .                                |                           +---------+  
    .                                |                           | IClient |                     
    .                                |                           +---------+              
    .                                |                               ^                    
    .                                |                               -                    
    .                                |                               |                    
    .                                |                               |                    
    .                                |                               |                    
    .                                |                           +------------+ 
    .                                +-------------------------->| MQTTClient |
    .                                |                           +------------+
    .                                |
    .                                |                           +---------+                                       
    .                                |                           | IServer |                     
    .                                |                           +---------+                    +--------------+   
    .                                |                               ^                          | IRestHandler |
    .                                |                               -                          +--------------+
    .                                |                               |                              ^
    .                                |                               |                              -
    .                                |                               |                              |
    .                                |                           +-----------------------+      +------------------+ 
    .                                +-------------------------->| WebSockHttpFlexServer |<-----| RestV1ApiHandler |
    .                                                            +-----------------------+      +------------------+
    .                                                                .
    .                                                                .
    ..................................................................
                                                                     .
                                                                     .
                                                                     .
                                                        +-----------------------+
                                                        |       grpcHandler     |
                                                        +-----------------------+

```
