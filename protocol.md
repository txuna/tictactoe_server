
# PROTOCOL 

### Login request
```
{
    "email" : STRING,
    "password" : STRING
}
```

### Login response 
```
{
    "user_id" : INTEGER, 
    "token" : STRING
}
```

### Register request
```
{
    "email" : STRING, 
    "password" : STRING, 
    "name" : STRING
}
```

### Register response
```
{
    "error" : INTERGER
}
```

### Logout request
```
{
    "token" : STRING, 
    "user_id" : INTEGER
}
```

### Logout response
```
{
    "error" : INTEGER
}
```

### Create Room request
```
{
    "token" : STRING, 
    "user_id" : INTEGER, 
    "title" : STRING
}
```

### Create Room response
```
{
    "error" : INTEGER
}
```

### Join Room request
```
{
    "token" : STRING, 
    "user_id" : INTEGER, 
    "room_id" : INTEGER
}
```

### Join Room response
```
{
    "error" : INTEGER
}
```

### Start Room request
```
{
    "token" : STRING
    "user_id" : INTEGER
    "room_id" : INTEGER
}
```

### Start Room response
```
{
    "error" : INTEGER
}
```

### Exit Room request
```
{
    "token" : STRING
    "user_id" : INTEGER
    "room_id" : INTEGER
}
```

### Exit Room response
```
{
    "error" : INTEGER
}
```