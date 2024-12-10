// A two-level bucket-heap priority queue for search_labels. 
// Higher level: Bucket queue
// Lower level: Heap queue
// First implementation pqueue_label_hybrid: Fixed range buckets (needs f_max-f_min)
// Second implementation pqueue_label_hybrid_cyclic: Cyclic buckets (needs bucket_size)
//
// Based on:
// 1979, Eric V. Denardo and Bennett L. Fox, Shortest-Route Methods: 1. Reaching, Pruning, and Buckets 
// @author: sahmadi
// @created: 04/06/2021
// @updated: 11/02/2022
//

#include <cassert>
#include <iostream>

#ifndef PQUEUE_LABEL_HYBRID_H
#define PQUEUE_LABEL_HYBRID_H

template <class Heap, class LABEL>
class pqueue_label_hybrid
{
    
	public:
    
        pqueue_label_hybrid(size_t size=1, cost_t fmin=0, cost_t fmax=0)
            : max_cursor_(0), queuesize_(0), cursor_(0), f_min(fmin), cmp(0)
        {
            high_level_ = new linkedlist<LABEL*>[1]();
            // bucket_width_ = size;
            bucket_width_ = (fmax - fmin)/1e6 + 1; // enforcing 1M bucket max
            initsize_ = (fmax - fmin)/bucket_width_ + 1;
            resize(initsize_);
        }

        ~pqueue_label_hybrid()
        {
            delete [] high_level_;
        }

        
        inline void
        range_update(cost_t fmin, cost_t fmax)
        {
            f_min = fmin;
        }
        
        void
        clear()
        {
            queuesize_ = 0;
            max_cursor_ = initsize_;
            delete [] high_level_;
            high_level_ = 0;
            cursor_ = 0;
        }

		// add a new element to the pqueue
        void 
        push (LABEL* lb)
        {
            size_t index = (lb->get_f_pri() - f_min)/bucket_width_;
            if(index == cursor_)
            (this->low_level_).push(lb);
            else
            (this->high_level_[index]).push_back(lb);
            
            queuesize_++;
            return;
        }

        // void 
        // pull_up(LABEL* lb, LABEL* lb_old)
        // {
        //     size_t index = (lb->get_f_pri() - f_min)/bucket_width_;
        //     size_t index_old = (lb_old->get_f_pri() - f_min)/bucket_width_;
        //     if (index == cursor_)
        //     {
        //         (this->low_level_).pull_up(lb, lb_old);
        //     }
        //     else
        //     {
        //         lb_old->set_f_pri(0);
        //         this->push(lb);
        //     }
        // }

		LABEL*
        pop()
        {
            if (queuesize_)
            {
                LABEL* ans = this->peek();
                (this->low_level_).pop();
                queuesize_--;
                return ans;
            }
            return 0;
        }

        // void 
        // pull_down(LABEL* lb)
        // {
        //     size_t index = (lb->get_f_pri() - f_min)/bucket_width_;
        //     if (index == cursor_)
        //     {
        //         (this->low_level_).pull_down(lb);
        //     }
        //     else
        //     {
        //         (this->high_level_[cursor_]).pop();
        //         this->push(lb);
        //     }
        // }

        inline LABEL *
        peek()
        {
            if (!low_level_.size())
            {
                while (cursor_ <= max_cursor_ && !high_level_[cursor_].size())
                {cursor_++;}

                while (high_level_[cursor_].size())
                {
                    (this->low_level_).push(high_level_[cursor_].front());
                    (this->high_level_[cursor_]).pop_front();
                    // cmp++;
                }
            }
            return this->low_level_.peek();
        }

        inline uint64_t
        get_cmp()
        {
            return cmp + cursor_ + (this->low_level_).get_cmp();
        }

        inline unsigned int
		size() const
		{
			return queuesize_;
		}

        void
        resize(uint newsize)
        {
            if(newsize - 1 <= max_cursor_)
            {
                return;
            }
            
            linkedlist<LABEL*>* tmp_ = new linkedlist<LABEL*>[newsize]();
            for(size_t i=0; i <= max_cursor_; i++)
	        {
                tmp_[i] = high_level_[i];
            }
            delete [] high_level_;
            high_level_ = tmp_;
            max_cursor_ = newsize-1;
            return;
        }

        size_t
		mem()
		{
			return max_cursor_*sizeof (LABEL*)
				+ sizeof(*this) + low_level_.mem();
		}

    private:
		size_t initsize_;
		size_t max_cursor_;
		unsigned int queuesize_;
		size_t cursor_;
        size_t bucket_width_;
        cost_t f_min;
        linkedlist<LABEL*>* high_level_;
        Heap low_level_;
        uint64_t cmp;
        
    
};

typedef pqueue_label_hybrid<pqueue_label_heap_min_w_tie, search_label_pri> pqueue_label_hybrid_min_w_tie;
typedef pqueue_label_hybrid<pqueue_label_heap_min_wo_tie, search_label_pri> pqueue_label_hybrid_min_wo_tie;
typedef pqueue_label_hybrid<pqueue_label_light_heap_min_w_tie, search_label_light_pri> pqueue_label_light_hybrid_min_w_tie;
typedef pqueue_label_hybrid<pqueue_label_light_heap_min_wo_tie, search_label_light_pri> pqueue_label_light_hybrid_min_wo_tie;


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
template <class Heap, class LABEL>
class pqueue_label_hybrid_cyclic
{
    
	public:
    
        pqueue_label_hybrid_cyclic(size_t size=1, cost_t fmin=0, cost_t fmax=0)
            : max_cursor_(1), queuesize_(0), cursor_(0), f_min(fmin), cmp(0)
        {
            // bucket_width_ = size;
            bucket_width_ = (fmax - fmin)/1e6 + 1;  // enforcing 1M bucket max
            offset_ = 0;
            high_level_ = new linkedlist<LABEL*>[1]();

        }

        ~pqueue_label_hybrid_cyclic()
        {
            delete [] high_level_;
        }

        
        inline void
        range_update(cost_t fmin, cost_t fmax)
        {
            f_min = fmin;
        }
        
        void
        clear()
        {
            queuesize_ = 0;
            max_cursor_ = initsize_;
            delete [] high_level_;
            high_level_ = 0;
            cursor_ = 0;
        }

		// add a new element to the pqueue
        void 
        push (LABEL* lb)
        {
            size_t index = (lb->get_f_pri() - f_min)/bucket_width_ + offset_;
            
            if(index >= max_cursor_)
            {
                resize(index * 2);
                index -= offset_;
                offset_ = 0;
                cursor_ = 0;
            }

            if(index == cursor_)
            {
                (this->low_level_).push(lb);
            }
            else
            {
                (this->high_level_[index % (max_cursor_)]).push_back(lb);
            }
            queuesize_++;
            return;
        }

        LABEL*
        pop()
        {
            if (queuesize_)
            {
                LABEL* ans = this->peek();
                (this->low_level_).pop();
                queuesize_--;
                return ans;
            }
            return 0;
        }

        inline LABEL *
        peek()
        {
            if (queuesize_)
            {
                if (!low_level_.size())
                {
                    while (!high_level_[cursor_ % max_cursor_].size())
                    {
                        cursor_++;
                        offset_++;
                        f_min += bucket_width_;
                    }

                    while (high_level_[cursor_ % max_cursor_].size())
                    {
                        (this->low_level_).push(high_level_[cursor_ % max_cursor_].front());
                        (this->high_level_[cursor_ % max_cursor_]).pop_front();
                        // cmp++;
                    }
                }
                return this->low_level_.peek();
            }
            return 0;
        }

        inline uint64_t
        get_cmp()
        {
            return cmp + cursor_ + (this->low_level_).get_cmp();
        }

        inline unsigned int
		size() const
		{
			return queuesize_;
		}

        void
        resize(uint newsize)
        {
            linkedlist<LABEL*>* tmp_ = new linkedlist<LABEL*>[newsize]();
            for(size_t i=0; i <= max_cursor_; i++)
	        {
                tmp_[i] = high_level_[(i + offset_) % max_cursor_];
            }
            delete [] high_level_;
            high_level_ = tmp_;
            max_cursor_ = newsize;
            return;
        }

        size_t
		mem()
		{
			return max_cursor_*sizeof (LABEL*)
				+ sizeof(*this) + low_level_.mem();
		}

    private:
		size_t initsize_;
		size_t max_cursor_;
		unsigned int queuesize_;
		size_t cursor_;
        size_t bucket_width_;
        uint64_t offset_;
        cost_t f_min;
        linkedlist<LABEL*>* high_level_;
        Heap low_level_;
        uint64_t cmp;
        
    
};

typedef pqueue_label_hybrid_cyclic<pqueue_label_heap_min_w_tie, search_label_pri> pqueue_label_hybrid_cyclic_min_w_tie;
typedef pqueue_label_hybrid_cyclic<pqueue_label_heap_min_wo_tie, search_label_pri> pqueue_label_hybrid_cyclic_min_wo_tie;
typedef pqueue_label_hybrid_cyclic<pqueue_label_light_heap_min_w_tie, search_label_light_pri> pqueue_label_light_hybrid_cyclic_min_w_tie;
typedef pqueue_label_hybrid_cyclic<pqueue_label_light_heap_min_wo_tie, search_label_light_pri> pqueue_label_light_hybrid_cyclic_min_wo_tie;

#endif