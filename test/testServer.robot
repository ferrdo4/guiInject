*** Settings ***
Library  Remote  http://localhost:8080
 
*** Test Cases ***

Ping Test
    ${pong} =                        ping  bob 
    Should Be Equal as Strings       ${pong}           pong bob

Objects test
   ${objs} =   read all objects
   log   ${objs}
 
Click object
    click   pushButton

Read property
    ${prop} =    read property     pushButton_2   visible 
    log   ${prop}

Set property
    set property     pushButton_2    visible    true

