*** Settings ***
Library  Remote  192.168.192.219:8888
 
*** Test Cases ***

Ping Test
    ${pong} =                        ping  bob 
    Should Be Equal as Strings       ${pong}           pong bob

Objects test
   ${objs} =   read all objects
   log   ${objs}


klik
    Click  ____KW_MainFrame___NextButton
    Click  NextButton
    Click  NextButton
    Click  NextButton
    Click  NextButton
