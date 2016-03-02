
# coding: utf-8

# In[75]:

import sys
import re
import random
from operator import itemgetter
from urllib2 import unquote
import urlparse

def prove(line, Answ):
    line = unquote(line)
    link = urlparse.urlparse(line)
    seg = filter(None, map(str.strip, link.path.split('/')))
    params = urlparse.parse_qs(link.query)
    
    st = "segments:" + str(len(seg))
    Answ.update({st : Answ.get(st,0) + 1})
    
    for i,j in params.items():
        st = "param_name:" + i
        Answ.update({st : Answ.get(st,0) + 1})
        st = "param:" + i + '=' + j[0]
        Answ.update({st : Answ.get(st,0) + 1})
        
    j = 0
    for i in seg:
        st = "segment_len_" + str(j) + ':' + str(len(i))
        Answ.update({st : Answ.get(st,0) + 1})
        st = "segment_name_" + str(j) + ':' + i
        Answ.update({st : Answ.get(st,0) + 1})
        
        param = i.split('.')
        f = 0
        if len(param) == 2:
            f = 1
            st = "segment_ext_" + str(j) + ':' + param[1]
            Answ.update({st : Answ.get(st,0) + 1})
            i = param[0]
        
        if i.isdigit():
            st = "segment_[0-9]_" + str(j) + ':1'
            Answ.update({st : Answ.get(st,0) + 1})
        else:              
            result = re.match('\D+\d+\D+',i)
            if result is not None:
                if f == 1:
                    st = "segment_ext_substr[0-9]_" + str(j) + ':1'
                    Answ.update({st : Answ.get(st,0) + 1})
                st = "segment_substr[0-9]_" + str(j) + ':1'
                Answ.update({st : Answ.get(st,0) + 1})
        #it's word
        j = j + 1


def extract_features(INPUT_FILE_1, INPUT_FILE_2, OUTPUT_FILE):
    f1 = open(INPUT_FILE_1, "r")
    f2 = open(INPUT_FILE_2, "r")
    f3 = open(OUTPUT_FILE, "w")
    random.seed()
    Answ = {}
    N = 1000  
    f1 = open(INPUT_FILE_1, 'r')
    qlink_urls = random.sample(f1.readlines(), N)
    f1.close()
    f2 = open(INPUT_FILE_2, 'r')
    qeneral_urls = random.sample(f2.readlines(), N)
    f2.close()
    for i in qlink_urls:
        prove(i,Answ)
    for i in qeneral_urls:
        prove(i,Answ)
            
    
    mAnsw = []
    for i in Answ:
         mAnsw.append([Answ.get(i),i])
    mAnsw = reversed(sorted(mAnsw))
    for i in mAnsw:
        if (i[0] >= 100):
            f3.write(str(i[1]) + '\t' + str(i[0]) + '\n')
    f3.close()
    f2.close()
    f1.close()

#extract_features('data/urls.lenta.examined', 'data/urls.lenta.examined', 'output.txt')
