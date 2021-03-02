#include <iostream>
#include <iomanip> 
#include <cmath>
#include <cstdlib>
#include <ctime>

#define MAX 100000 
#define	DRAM_SIZE (64*1024*1024)

using namespace std ;

int compulsory_misses=0, capacity_misses=0, conflict_misses=0 ;

unsigned int GenerateAddresses()
{
    static unsigned int addr=0;
    return (addr++)%(DRAM_SIZE);
}

bool cacheSimulator(int cache_type,unsigned int addr,int cache[3][100000],int type,int &block_counter,int index_addr,int tag_addr,int num_of_blocks, int block_size){
	
	bool detected = 0 ;
	int shift_offset = log2(block_size) ;
	bool flag_miss = 1 ;
	
	//Direct Addressing
	if (cache_type == 0){
			if (cache[0][index_addr]==tag_addr){
				return 1 ;
			}
			
			else{
				cache[0][index_addr] = tag_addr;
                for (int i=0 ; i<num_of_blocks ; i++)
                {
                    if (cache[1][i] != 1){   
						flag_miss = false;
                        i = num_of_blocks;
					}    
                }
                
                if (flag_miss)
                    capacity_misses++ ;   
                else
                {
                    if(cache[1][index_addr]==1)
                        conflict_misses++;
                    else
                        compulsory_misses++;
                }
                cache[1][index_addr]= 1;
                return 0;		
		}
	}
	
	//Set Associative
	else if (cache_type == 1){

        index_addr = index_addr * type;
        for (int i=0; i<type ; i++)
        {
            if (cache[0][index_addr+i] == tag_addr){ return 1;}
        }
        for (int j=0; j < type; j++)
        {
            if (cache[1][index_addr+j] == -1)
            {
                compulsory_misses++;
                cache[0][index_addr+j]=tag_addr;
                cache[1][index_addr+j]=1;
                return 0;
            }
        }
        
        srand(time(NULL));
        int x=rand()%(type);
        cache[0][index_addr+x]=tag_addr;
        cache[1][index_addr+x]=1;
        capacity_misses++;
        return 0;
		
	}
	
	//Fully Associative (FIFO)
	else if (cache_type == 2){
			if(block_counter < num_of_blocks){
				for(int i=0 ; i<num_of_blocks ; i++){
					if(cache[0][i] == addr >> shift_offset ){
						detected = true ;  //cache hit
						block_counter--; 
						return detected;
					}
				}
				//cache miss
				if(!detected){
					cache[0][block_counter]=addr>>shift_offset;
                    cache[1][block_counter]=block_counter;
                    compulsory_misses++;
                    return false;
				}
			}
			//block more than block size
			else{
				for (int i=0; i < num_of_blocks; i++){
                    if (cache[0][i]==(addr >> shift_offset))
                    {
                        detected=true;
                        return detected;
                    }
                }
                
                if (!detected){
                    int compare=0;
                    for (int i=1; i < num_of_blocks; i++){
                        if (cache[1][compare] > cache[1][i]) //hit
                            compare=i;
                    }
                    
                    cache[0][compare]=addr >> shift_offset;
                    cache[1][compare]=block_counter;
                    capacity_misses++;
                    return false;  //cache miss
                }
			}		
	}
	return true ;
}

int main(){
	
	int cache_type, block_size, cache_size, number_of_blocks=0 ;
	
	int hit_counter=0 , tag_addr=0 , index_addr=0 , block_counter=0 ;
	
	int addr, shift, result ;
	
	char *for_comparision[2] = {"Miss","Hit"};
	
	int cache[3][MAX] ;
	
    cout <<"0 ------- Direct mapped"<<endl<<"1 ------- Set associative"<<endl<<"2 ------- Fully associative"<< endl ;
    cout<<"Enter your choice : ";
    cin>>cache_type ;
    
    cout<<endl<<"Enter size of block (power of 2 between 4 and 128 bytes)"<<endl ;
    cin>>block_size ;
    
    cout<<endl<<"Enter cache size (1KB to 64KB)"<<endl ;
    cin>>cache_size ;
    
    switch(cache_type){
    	
    	//Direct Mapped
    	case 0 :
    		
    		number_of_blocks = (cache_size*1024)/block_size ;
    		
    		for (int i=0 ; i<2 ; i++){
    			for (int j=0 ; j<number_of_blocks ; j++){
    				cache[i][j] = -1 ;
				}
			}
			
	        for(int i=0 ; i<100000 ; i++)
	        {
	            addr = GenerateAddresses() ;
	            
	            shift = log2(block_size) ;
	            index_addr = (addr >> shift)% number_of_blocks ;
	            
	            shift = log2(number_of_blocks+block_size) ;
	            tag_addr = addr>>shift ;  
	            
	            result = cacheSimulator(cache_type, addr, cache, 0, block_counter, index_addr, tag_addr, number_of_blocks, block_size);
	            
	            //cout <<"0x" << setfill('0') << setw(8) << hex << addr <<" ("<< for_comparision[result] <<")\n";
	            
            	index_addr = 0;
            	tag_addr = 0;	            

	            if (for_comparision[result] == "Hit"){
	                hit_counter++;
	            }
	        }			
    		
    		cout<<endl<<"Hits : "<<hit_counter<<endl ;
			cout<<"Compulsory Misses : "<< compulsory_misses<<endl ;
			cout<<"Capcity Misses: " << capacity_misses<<endl ;
			cout<<"Conflict Misses: " << conflict_misses<<endl;	
    		break ;
    	
    	//Set Associative
    	case 1 :
    		
    		int set_size ;
    		
    		cout<<endl<<"Enter the number of ways for the set associative cache : 2 , 4 , 8 , 16 "<<endl ;
    		cin>>set_size ;
    		
    		number_of_blocks = (cache_size*1024)/(block_size*set_size) ;
    		
    		for (int i=0 ; i<3 ; i++){
    			for (int j=0 ; j<number_of_blocks ; j++){
    				cache[i][j] = -1 ;
				}
			}   
			
	        for(int i=0 ; i <100000 ; i++, index_addr=0, tag_addr=0)
	        {
	            addr = GenerateAddresses();
	            
	            shift= log2(block_size);
	            index_addr= (addr >> shift)% (number_of_blocks);
	            
	            shift= log2(number_of_blocks+block_size);
	            tag_addr= addr >>shift;
	            
	            result = cacheSimulator(cache_type, addr, cache, set_size, block_counter, index_addr, tag_addr, number_of_blocks, block_size) ;
	            
	            //cout <<"0x" << setfill('0') << setw(8) << hex << addr <<" ("<< for_comparision[result] <<")\n";
	            
	            if (for_comparision[result] == "Hit"){
	                hit_counter++;
	            }
	            
	            block_counter++;
	        }						 		
    		
    		cout<<endl<<"Hits : "<< hit_counter<<endl ;
			cout<<"Compulsory Misses : "<< compulsory_misses<<endl ;
			cout<<"Capcity Misses: " << capacity_misses<<endl ;
			cout<<"Conflict Misses: " << conflict_misses<<endl;	    		
    		break ;
    		
    	//Fully Associative	
    	case 2 :
    		
    		number_of_blocks= (cache_size*1024)/block_size;
    		
    		for (int i=0 ; i<3 ; i++){
    			for (int j=0 ; j<number_of_blocks ; j++){
    				cache[i][j] = -1 ;
				}
			}
			
	        for(int i=0 ; i<100000 ; i++)
	        {
	            addr = GenerateAddresses() ;
	            
	            result = cacheSimulator(cache_type, addr, cache, 0, block_counter, index_addr, tag_addr, number_of_blocks, block_size);
	            
	            //cout <<"0x" << setfill('0') << setw(8) << hex << addr <<" ("<< for_comparision[result] <<")\n";
	           
	            if (for_comparision[result] == "Hit"){
	                hit_counter++;
	            }
	            
	            block_counter++;
	        }    		
			
    		cout<<endl<<"Hits : "<< hit_counter<<endl ;
			cout<<"Compulsory Misses : "<< compulsory_misses<<endl ;
			cout<<"Capcity Misses: " << capacity_misses<<endl ;
			cout<<"Conflict Misses: " << conflict_misses<<endl;	
    		break ;   	
	}
	
	return 0 ;
}
