# Protocol 

### Register 
request
```
{
    "email" : STRING, 
    "password" : STRING
}
```
response
```
{
    "error" : INTEGER
}
```

### Login 
request
```
{
    "email" : STRING, 
    "password" : STRING
}
```
response
```
{
    "error" : INTEGER
}
```

### Logout
request
```
{
    "user_id" : INTEGER, 
    "token" : STRING,
}
```
response
```
{
    "error" : INTEGER
}
```

### CreateRoom 
request
```
{
    "user_id" : INTEGER, 
    "token" : STRING,
    "title" : STRING
}
```
response
```
{
    "error" : INTEGER
}
```

### Join Room
request
```
{
    "user_id" : INTEGER, 
    "token" : STRING,
    "room_id" : INTEGER
}
```
response (JOIN시 참여하려는 방의 인원 정보를 받아옴)
```
{
    "error" : INTEGER, 
    "player" : {
        "user_id" : INTEGER, 
        "name" : STRING
    }
}
```

### PlayerJoinRoomResponse
response (상대방이 JOIN일 때 호스트에게 JOIN하려는 대상의 정보를 전송함)
```
{
    "error" : INTEGER, 
    "player" : {
        "user_id" : INTEGER, 
        "name" : STRING
    }
}
```

### StartRoom
request
```
{
    "user_id" : INTEGER, 
    "token" : STRING,
}
``` 
response (방에 참가한 상대방에게도 같은 메시지 전달)
```
{
    "error" : INTEGER, 
    "who_is_turn" : INTEGER /* When Error is None */
}
```

### ExitRoom
request
```
{
    "user_id" : INTEGER, 
    "token" : STRING,
}
```
response (방에 참가하는 또 다른 사람이 존재할시)
```
{
    "error" : INTEGER
}
```

### PlayerExitRoomResponse
(방에 참가하고 있는 다른 사람이 나갈 시 현재 방에 참가하고 있는 이에게 전송)
response
```
{
    "error" : INTEGER, 
    "user_id" : INTEGER
}
```

### LoadRoom
request
```
{
    "user_id" : INTEGER, 
    "token" : STRING
}
```
response
```
{
    "error" : INTEGER, 
    "rooms" : [
        {
            "room_id" : INTEGER, 
            "state" : INTEGER, 
            "host_id" : INTEGER, 
            "other_id" : INTEGER,
            "title" : STRING, 
            "is_start" : BOOLEAN
        },
        {
            ...
        }
    ]
}
```

### PlayerTurn
request 
```
{
    "user_id" : INTEGER, 
    "token" : STRING,
    "position" : INTEGER
}
```
response (방에 속한 모두에게 Response를 전달한다. )
```
{
    "error" : INTEGER,
    "turn_user_id" : INTEGER, /* Request를 보낸 User ID  */
    "position" : INTEGET, /* request를 보낸 user가 놓은 위치 */
    "is_fin" : BOOLEAN, /* 게임 끝났는지 */
    "who_is_win" : INTEGER, /* 게임이 끝났을 때 추가되는 값  - DRAW_GAME 0  */
    "who_is_turn" : INTEGER
}
```