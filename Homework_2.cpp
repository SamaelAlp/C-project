#include <iostream>

// Шаблонный класс Map, где Key - тип ключа, T - тип хранимых данных
template <typename Key, typename T>
class Map {
private:
    // Цвета узлов: красный и чёрный
    enum Color { RED, BLACK };
    
    // Структура для представления узла дерева
    struct Node {
        Key key;        // Ключ элемента
        T value;        // Значение элемента
        Node* left;     // Указатель на левого потомка
        Node* right;    // Указатель на правого потомка
        Node* parent;   // Указатель на родительский узел
        Color color;    // Цвет узла (красный или чёрный)
        
        // Конструктор узла: инициализирует все поля
        Node(const Key& k, const T& v) 
            : key(k), value(v), left(nullptr), right(nullptr), parent(nullptr), color(RED) {}
    };
    
    Node* root;  // Корень дерева
    Node* nil;   // Специальный "нулевой" узел (представляет листья дерева)

    // Вспомогательные функции для балансировки дерева

    // Левый поворот вокруг узла x
    void left_rotate(Node* x) {
        Node* y = x->right;        // y становится правым потомком x
        x->right = y->left;        // Левое поддерево y становится правым поддеревом x
        if (y->left != nil) {      // Если у y есть левый потомок
            y->left->parent = x;   // Устанавливаем родителя для левого потомка y
        }
        y->parent = x->parent;     // Родитель y теперь такой же, как у x
        if (x->parent == nullptr) { // Если x был корнем
            root = y;              // y становится новым корнем
        } else if (x == x->parent->left) { // Если x был левым потомком
            x->parent->left = y;   // y становится левым потомком родителя x
        } else {                   // Если x был правым потомком
            x->parent->right = y;  // y становится правым потомком родителя x
        }
        y->left = x;               // x становится левым потомком y
        x->parent = y;             // Устанавливаем y как родителя x
    }

    // Правый поворот (аналогично левому, но в другую сторону)
    void right_rotate(Node* x) {
        Node* y = x->left;         // y становится левым потомком x
        x->left = y->right;        // Правое поддерево y становится левым поддеревом x
        if (y->right != nil) {     // Если у y есть правый потомок
            y->right->parent = x;  // Устанавливаем родителя для правого потомка y
        }
        y->parent = x->parent;     // Родитель y теперь такой же, как у x
        if (x->parent == nullptr) { // Если x был корнем
            root = y;              // y становится новым корнем
        } else if (x == x->parent->right) { // Если x был правым потомком
            x->parent->right = y;  // y становится правым потомком родителя x
        } else {                   // Если x был левым потомком
            x->parent->left = y;   // y становится левым потомком родителя x
        }
        y->right = x;              // x становится правым потомком y
        x->parent = y;             // Устанавливаем y как родителя x
    }

    // Исправление дерева после вставки нового узла
    void fix_insert(Node* k) {
        // Пока родитель нового узла красный (нарушает свойство дерева)
        while (k->parent != nullptr && k->parent->color == RED) {
            // Если родитель является левым потомком своего родителя
            if (k->parent == k->parent->parent->left) {
                Node* uncle = k->parent->parent->right; // Дядя - правый потомок дедушки
                // Если дядя красный
                if (uncle->color == RED) {
                    k->parent->color = BLACK;    // Родитель становится чёрным
                    uncle->color = BLACK;        // Дядя становится чёрным
                    k->parent->parent->color = RED; // Дедушка становится красным
                    k = k->parent->parent;       // Перемещаемся к дедушке для проверки выше
                } else {
                    // Если новый узел является правым потомком
                    if (k == k->parent->right) {
                        k = k->parent;           // Поднимаемся к родителю
                        left_rotate(k);           // Делаем левый поворот
                    }
                    k->parent->color = BLACK;    // Родитель становится чёрным
                    k->parent->parent->color = RED; // Дедушка становится красным
                    right_rotate(k->parent->parent); // Правый поворот вокруг дедушки
                }
            } 
            // Случай, когда родитель является правым потомком своего родителя (аналогично предыдущему)
            else {
                Node* uncle = k->parent->parent->left; // Дядя - левый потомок дедушки
                if (uncle->color == RED) {
                    k->parent->color = BLACK;
                    uncle->color = BLACK;
                    k->parent->parent->color = RED;
                    k = k->parent->parent;
                } else {
                    if (k == k->parent->left) {
                        k = k->parent;
                        right_rotate(k);
                    }
                    k->parent->color = BLACK;
                    k->parent->parent->color = RED;
                    left_rotate(k->parent->parent);
                }
            }
        }
        root->color = BLACK; // Корень всегда должен быть чёрным
    }

    // Находит узел с минимальным ключом в поддереве
    Node* minimum(Node* node) const {
        // Идем влево, пока есть левые потомки
        while (node->left != nil) {
            node = node->left;
        }
        return node; // Самый левый узел - минимальный
    }

    // Заменяет поддерево с корнем u поддеревом с корнем v
    void transplant(Node* u, Node* v) {
        if (u->parent == nullptr) { // Если u - корень
            root = v; // v становится новым корнем
        } else if (u == u->parent->left) { // Если u - левый потомок
            u->parent->left = v; // v становится левым потомком родителя u
        } else { // Если u - правый потомок
            u->parent->right = v; // v становится правым потомком родителя u
        }
        v->parent = u->parent; // Устанавливаем родителя для v
    }

    // Исправление дерева после удаления узла
    void fix_delete(Node* x) {
        // Пока x не корень и его цвет чёрный (нарушение свойств)
        while (x != root && x->color == BLACK) {
            // Если x - левый потомок
            if (x == x->parent->left) {
                Node* w = x->parent->right; // w - брат x (правый потомок родителя)
                // Если брат красный
                if (w->color == RED) {
                    w->color = BLACK; // Брат становится чёрным
                    x->parent->color = RED; // Родитель становится красным
                    left_rotate(x->parent); // Левый поворот вокруг родителя
                    w = x->parent->right; // Обновляем брата
                }
                // Если оба потомка брата чёрные
                if (w->left->color == BLACK && w->right->color == BLACK) {
                    w->color = RED; // Брат становится красным
                    x = x->parent; // Поднимаемся к родителю
                } else {
                    // Если правый потомок брата чёрный
                    if (w->right->color == BLACK) {
                        w->left->color = BLACK; // Левый потомок брата чёрный
                        w->color = RED; // Брат становится красным
                        right_rotate(w); // Правый поворот вокруг брата
                        w = x->parent->right; // Обновляем брата
                    }
                    w->color = x->parent->color; // Цвет брата = цвет родителя
                    x->parent->color = BLACK; // Родитель становится чёрным
                    w->right->color = BLACK; // Правый потомок брата чёрный
                    left_rotate(x->parent); // Левый поворот вокруг родителя
                    x = root; // x становится корнем
                }
            } 
            // Случай, когда x - правый потомок (аналогично предыдущему)
            else {
                Node* w = x->parent->left;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    right_rotate(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == BLACK && w->left->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->left->color == BLACK) {
                        w->right->color = BLACK;
                        w->color = RED;
                        left_rotate(w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->left->color = BLACK;
                    right_rotate(x->parent);
                    x = root;
                }
            }
        }
        x->color = BLACK; // x всегда должен быть чёрным
    }

    // Рекурсивное удаление всего дерева
    void clear(Node* node) {
        if (node != nil) { // Если узел не является nil (не пустой)
            clear(node->left);  // Удаляем левое поддерево
            clear(node->right); // Удаляем правое поддерево
            delete node;        // Удаляем сам узел
        }
    }

public:
    // Конструктор: инициализирует пустое дерево
    Map() {
        nil = new Node(Key(), T()); // Создаем специальный nil-узел
        nil->color = BLACK; // nil-узел всегда чёрный
        nil->left = nil->right = nil->parent = nil; // Все указатели в nil указывают на самого себя
        root = nil; // Дерево пустое, корень указывает на nil
    }

    // Деструктор: освобождает память
    ~Map() {
        clear(root); // Удаляем всё дерево
        delete nil;  // Удаляем специальный nil-узел
    }

    // Добавляет элемент в дерево
    void insert(const Key& key, const T& value) {
        Node* new_node = new Node(key, value); // Создаем новый узел
        new_node->left = nil; // Левый потомок указывает на nil
        new_node->right = nil; // Правый потомок указывает на nil
        Node* parent = nullptr; // Родитель нового узла (пока неизвестен)
        Node* current = root; // Начинаем с корня

        // Поиск места для вставки
        while (current != nil) {
            parent = current; // Запоминаем текущий узел как потенциального родителя
            // Сравниваем ключи для определения направления движения
            if (key < current->key) {
                current = current->left; // Двигаемся влево
            } else if (key > current->key) {
                current = current->right; // Двигаемся вправо
            } else {
                // Если ключ уже существует, обновляем значение и выходим
                current->value = value;
                delete new_node; // Удаляем созданный узел
                return;
            }
        }

        new_node->parent = parent; // Устанавливаем родителя для нового узла
        // Если дерево пустое (родитель не найден)
        if (parent == nullptr) {
            root = new_node; // Новый узел становится корнем
        } 
        // Если ключ меньше ключа родителя
        else if (key < parent->key) {
            parent->left = new_node; // Новый узел становится левым потомком
        } 
        // Если ключ больше ключа родителя
        else {
            parent->right = new_node; // Новый узел становится правым потомком
        }

        // Корень всегда должен быть чёрным
        if (new_node->parent == nullptr) {
            new_node->color = BLACK;
            return;
        }

        // Если у родителя нового узла нет родителя (он корень), балансировка не нужна
        if (new_node->parent->parent == nullptr) {
            return;
        }

        // Исправляем дерево после вставки
        fix_insert(new_node);
    }

    // Ищет элемент по ключу
    T* find(const Key& key) {
        Node* current = root; // Начинаем с корня
        while (current != nil) { // Пока не достигнем конца дерева
            if (key < current->key) {
                current = current->left; // Двигаемся влево
            } else if (key > current->key) {
                current = current->right; // Двигаемся вправо
            } else {
                return &(current->value); // Нашли элемент, возвращаем указатель на значение
            }
        }
        return nullptr; // Элемент не найден
    }

    // Удаляет элемент по ключу
    void erase(const Key& key) {
        Node* z = root; // Начинаем поиск с корня
        // Поиск узла с заданным ключом
        while (z != nil) {
            if (key < z->key) {
                z = z->left; // Двигаемся влево
            } else if (key > z->key) {
                z = z->right; // Двигаемся вправо
            } else {
                break; // Нашли узел
            }
        }
        if (z == nil) return; // Элемент не найден, выходим

        Node* y = z; // y - узел, который будет удален
        Node* x; // x - узел, на место которого будет перемещен y
        Color y_original_color = y->color; // Сохраняем исходный цвет узла y

        // Случай 1: Узел имеет 0 или 1 потомка
        if (z->left == nil) {
            x = z->right; // x - правый потомок z
            transplant(z, z->right); // Заменяем z на его правого потомка
        } 
        // Случай 2: Узел имеет только левого потомка
        else if (z->right == nil) {
            x = z->left; // x - левый потомок z
            transplant(z, z->left); // Заменяем z на его левого потомка
        } 
        // Случай 3: Узел имеет двух потомков
        else {
            y = minimum(z->right); // Находим минимальный узел в правом поддереве
            y_original_color = y->color; // Сохраняем цвет найденного узла
            x = y->right; // Запоминаем правого потомка y
            
            // Если y является прямым потомком z
            if (y->parent == z) {
                x->parent = y; // Устанавливаем родителя для x
            } 
            // Если y находится глубже
            else {
                transplant(y, y->right); // Удаляем y из дерева
                y->right = z->right; // Правое поддерево z становится правым поддеревом y
                y->right->parent = y; // Устанавливаем родителя
            }
            
            transplant(z, y); // Заменяем z на y
            y->left = z->left; // Левое поддерево z становится левым поддеревом y
            y->left->parent = y; // Устанавливаем родителя
            y->color = z->color; // Устанавливаем цвет y как у z
        }

        delete z; // Удаляем исходный узел
        
        // Если удаленный узел был чёрным, нужно исправить дерево
        if (y_original_color == BLACK) {
            fix_delete(x);
        }
    }
};

// Пример использования
int main() {
    // Создаем с ключами типа int и значениями типа string
    Map<int, std::string> m;
    
    // Добавляем элементы
    m.insert(10, "ten");
    m.insert(20, "twenty");
    m.insert(5, "five");
    
    // Ищем элемент с ключом 10
    auto val = m.find(10);
    if (val) std::cout << *val << std::endl; // Выведет "ten"
    
    // Удаляем элемент с ключом 5
    m.erase(5);
    
    return 0;
}


/* Erase — функция, которая удаляет элементы из контейнеров, например, из вектора.
Enum (enumeration, перечисление) — пользовательский тип данных, состоящий из набора именованных констант (перечислителей).
nullptr — ключевое слово, которое указывает на отсутствие значения (нулевой указатель). 
auto — ключевое слово позволяет компилятору автоматически определять тип переменной, тип возвращаемого значения функции или параметра шаблона на основе контекста, в котором они используются.
*/
