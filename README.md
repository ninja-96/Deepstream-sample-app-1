# Deepstream sample app 1 (С++)

Пайплайн на Gstreamer с использование плагинов Deepstream\

# Зависимости
- Gstreamer 1.14 и выше
- Deepstream 6.0 и выше

## Сборка проекта
```
mkdir build && cd build
cmake ..
make -j8
```
Проект расчитан на использование на платформе Nvidia Jetson.\
Для использования на x86-совместимой платформе могут потребоваться изменения в [CMake-файле](./CMakeLists.txt).

## Запуск
```
./deepstream_cxx <путь к видеофайлу> <путь к конфигурационному файлу deepstream>
```
Пример конфигурационного файла можно найти [тут](./deepstream.cfg)


# Описание 
## Пайплайн
filesrc -> qtdemux -> h264parse -> nvv4l2decoder -> nvstreammux -> nvinfer -> fakesink\
В [main.cpp](./main.cpp) можно найти базовый пример для работы с Gstreamer и Deepstream.\
Программа считывает видео из файла, отделяет видеопоток от аудиопотока, парсит и декодирует raw-поток данных и отправляет полученные изображения на детектор (**nvinfer**). На **fakesink** получаем данные от детектора (bbox, conf, class) с помощью *gst_pad_add_probe*, которые потом можно использовать для любых целей.

В результате выполнения в консоль будет выводится информация следующего типа:
```
Frame Number = 0 Number of objects = 1 Vehicle Count = 0 Person Count = 1
Frame Number = 1 Number of objects = 1 Vehicle Count = 0 Person Count = 1
Frame Number = 2 Number of objects = 1 Vehicle Count = 0 Person Count = 1
Frame Number = 3 Number of objects = 1 Vehicle Count = 0 Person Count = 1
Frame Number = 4 Number of objects = 1 Vehicle Count = 0 Person Count = 1
Frame Number = 5 Number of objects = 1 Vehicle Count = 0 Person Count = 1
Frame Number = 6 Number of objects = 1 Vehicle Count = 0 Person Count = 1
Frame Number = 7 Number of objects = 1 Vehicle Count = 0 Person Count = 1
Frame Number = 8 Number of objects = 1 Vehicle Count = 0 Person Count = 1
Frame Number = 9 Number of objects = 1 Vehicle Count = 0 Person Count = 1
Frame Number = 10 Number of objects = 1 Vehicle Count = 0 Person Count = 1
Frame Number = 11 Number of objects = 1 Vehicle Count = 0 Person Count = 1
Frame Number = 12 Number of objects = 1 Vehicle Count = 0 Person Count = 1
```
