CXX=g++
DEF= -g -O2
INC= -I/usr/include -I. -I../../sampling
LIB= -L/usr/lib -L.
bin= learner eval libmlr.so
OBJ= listnet.o listmle.o ensemble.o instance.o eval.o

all: $(bin)

eval: eval.o libmlr.so
	$(CXX) $(DEF) $(INC) $(LIB) $< -o $@ -lmlr

learner: main.o libmlr.so
	$(CXX) $(DEF) $(INC) $(LIB) $< -o $@ -lmlr

libmlr.so: $(OBJ)
	$(CXX) $(DEF) $(INC) $(LIB) -shared $(OBJ) -o $@

.cpp.o:
	$(CXX) $(DEF) $(INC) $(LIB) -fPIC -c $<

litnet.o: listnet.hpp learner.hpp ranker.hpp
listmle.o: listnet.hpp listmle.hpp learner.hpp ranker.hpp
ensemble.o: ensemble.hpp
instance.o: instance.hpp

clean:
	@rm $(bin) $(OBJ)
