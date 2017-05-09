*** Settings ***
Library  Remote  192.168.192.219:8888
 
*** Test Cases ***

Ping Test
    ${pong} =                        ping  bob 
    Should Be Equal as Strings       ${pong}           pong bob
    start pick

Objects test
   ${objs} =   read all objects
   log   ${objs}


klik
    @{lst} =  Create List  Next  Button
    ${pth} =  Find Path  ${lst}
    Log  ${pth}
    Click  ____KW_MainFrame___NextButton
    Click  NextButton
    Click  NextButton
    Click  NextButton
    Click  NextButton
