# Gui Inject

>library to be injected with QT application to be able to click on gui with robot framework test

|**Keyword**|**Description**|**Params**|
|---|---|---|
|ping|test server response|string to return|
|read all objects|return list of all QObjects| --- |
|click|click into a widget|object name|
|read property|read property of a widget|object, property name|
|set property|set property to a widget|object, property name, value|
|set combobox index|set index of selected item in combobox|object name, index|
|find path|find path to object containing strings|string list|
|start pick| start pick mode|---|
|stop pick|stop pick mode|---|
|refresh|refresh object map|---|

thanks to :
>[Qt-Inspector](https://github.com/robertknight/Qt-Inspector.git)<br>
>[libmaia](https://github.com/wiedi/libmaia.git)
