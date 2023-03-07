#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include "Log.h"

namespace File
{

    class File
    {
    public:
        char *Read(const char *filename)
        {
            std::ifstream file(filename, std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                // Tính kích thước file
                file.seekg(0, std::ios::end);
                std::streamsize file_size = file.tellg();
                file.seekg(0, std::ios::beg);

                // Đọc nội dung file
                char *file_content = new char[file_size];
                file.read(file_content, file_size);
                file.close();
                return file_content;
            }
            else
            {
                LOGW("Open file error");
            }
            return NULL;
        }

        void Write(const char *filename, char *content)
        {
            if (content)
            {
                // Ghi nội dung file vào file ở thư mục hiện tại
                std::ofstream file(filename, std::ios::out | std::ios::binary);
                file.write((char *)content, strlen(content));
                file.close();
            }
            else
            {
                LOGW("Data error");
            }
        }

        void ZipFile(const char * filename)
        {
            // system()
        }
    };
}