# My_Shell

### Реализовано:

- конвейер;

- перенаправление ввода-вывода;

- запуск в фоновом режиме;

- &&;

- выполнение команд, указанных в круглых скобках в рамках дочернего процесса.

### Примеры работы:
#### $ ls | cat -n

#### $ yes my_shell | head

#### $ sleep 3 & pwd


### Структуры:

main.c - основная часть программы, из нее происходит вызов всех вспомогательных функций;

list.c - получает на вход строку символов и строит из нее список слов по всем правилам синтаксиса;

tree.c - получает на вход список слов и строит из него дерево, по которому будет затем выполнять работа shell;

exec.c - по построенному дереву непосредвенно выполняет команды shell;
