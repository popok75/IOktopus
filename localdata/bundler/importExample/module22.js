import { mod222 } from './module222.js';

 function mod22(x, y) {
    return x+y+mod222(x,y);
};

   
 function mod23(x, y) {
    return x+y;
};
 
 var origexpor=export;
  
 export(mod22);
 
}
 
