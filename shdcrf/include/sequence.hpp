# ifndef __SEQUENCE_H__
# define __SEQUENCE_H__

# include <vector>
# include <string>
# include <cstdlib>
# include <iostream>
# include <fstream>

namespace sequential
{
   static const char *bos[] = {"B_1", "B_2", "B_3", "B_4", "B_5"};
   static const char *eos[] = {"E_1", "E_2", "E_3", "E_4", "E_5"};

   static inline bool IsEOS(const char *str)
   {
      if (std::strcmp(str,"") == 0)
      {
         return true;
      }
      return false;
   }
   static inline bool IsEOS(std::string& s)
   {
      if (s == "")
      {
         return true;
      }
      return false;
   }
   static inline void chomp(char *str)
   {
      int len = std::strlen(str);
      if (*(str+len-1) == '\n')
      {
         *(str+len-1) = '\0';
         --len;
      }
      if (*(str+len-1) == '\r')
      {
         *(str+len-1) = '\0';
      }
   }
   static inline void chomp(std::string& s)
   {
      int len = s.size();
      if (s[len-1] == '\n')
      {
         s[len-1] = '\0';
         --len;
      }
      if (s[len-1] == '\r')
      {
         s[len-1] = '\0';
      }
   }

   class sequence
   {
      public:
         sequence();
         virtual ~sequence();
         void push(const char *str);
         const char* get(int row, int col);
         void clear();
         void dump();
         int getr() { return this->rows; };
         int getc() { return this->cols; };
      protected:
         std::vector<std::string> tokens;
         int cols;
         int rows;
         bool init;

         int setcols(const char *str);
      private:
         sequence(const sequence&);
         sequence& operator=(const sequence&);
   };

   inline sequence::sequence()
   : cols(0), rows(0), init(false)
   {
   }

   inline sequence::~sequence()
   {
   }

   inline void sequence::clear()
   {
      this->cols = 0;
      this->rows = 0;
      this->tokens.clear();
      this->init = false;
   }

   inline void sequence::dump()
   {
      for (int i = 0; i < this->rows; ++i)
      {
         for (int j = 0; j < this->cols; ++j)
         {
            std::cout << this->tokens[i*(this->cols)+j];
            if (j == this->cols-1)
            {
               std::cout << std::endl;
            }
            else
            {
               std::cout << "\t";
            }
         }
      }
   }

   inline const char* sequence::get(int r, int c)
   {
      if (c >= this->cols || c < 0)
      {
         throw "ERR: invalid pointer";
      }
      else if (r >= this->rows)
      {
         int id = r - this->rows;
         if (id > 4)
         {
            return eos[4];
         }
         return eos[id];
      }
      else if (r < 0)
      {
         int id = -r -1;
         if (id > 4)
         {
            return bos[4];
         }
         return bos[id];
      }
      return this->tokens[r*(this->cols)+c].c_str();
   }

   inline void sequence::push(const char *str)
   {
      if (!this->init)
      {
         this->cols = this->setcols(str);
         this->init = true;
      }
      else if (this->setcols(str) != this->cols)
      {
         std::string ex("ERR: found unmatched row ");
         ex += str;
         throw ex.c_str();
      }
      std::string c(str);
      for (int i = 0, p = 0; i < c.size()+1; ++i)
      {
         if (c[i] == '\t' || c[i] == '\0')
         {
            std::string token(c, p, i-p);
            this->tokens.push_back(token);
            p = i+1;
         }
      }
      ++this->rows;
   }

   inline int sequence::setcols(const char *str)
   {
      if (!str)
      {
         throw "ERR: str = Null";
      }
      int s = 1;
      for (const char *p = str; *p != '\0'; ++p)
      {
         if (*p == '\t')
         {
            ++s;
         }
      }
      return s;
   }

   static void sqread(FILE *fp, sequence *s, unsigned int bufsize)
   {
      char buf[bufsize];
      while (fgets(buf, bufsize, fp) != NULL)
      {
         chomp(buf);
         if (IsEOS(buf))
         {
            break;
         }
         s->push(buf);
      }
   }
   static void sqread(std::ifstream& in, sequence *s)
   {
      std::string line;
      while (std::getline(in, line))
      {
         chomp(line);
         if (IsEOS(line))
         {
            break;
         }
         s->push(line.c_str());
      }
   }
}
# endif /* __SEQUENCE_H__ */
