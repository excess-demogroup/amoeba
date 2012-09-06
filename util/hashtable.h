#ifndef _HASH_H
#define _HASH_H 1

#define BUCKET_BITS 4
#define NUM_BUCKETS (1<<BUCKET_BITS)
#define BUCKET_MASK (NUM_BUCKETS-1)
#define CASE_INSENSITIVE_HASHES 1

typedef void (*hash_callback)(const char * const key,
	      	              const void * const obj,
		   	      const void * const user_data);

class Hashtable {
protected:
	static unsigned int string_hash(const char *str);

	struct linked_list {
		char *key;
		unsigned int hash;
		void *obj;
		struct linked_list *next;
	};
	linked_list *buckets[NUM_BUCKETS];

public:
	Hashtable();
	Hashtable(Hashtable *h);
	~Hashtable();

	void insert(const char * const key, void * const obj);
	void insert(const char * const key, char * const str);
	void remove(const char * const key);
	void *lookup(const char * const key) const;
	void destroy_values();

	bool exists(const char * const key) const;
	char *get_str(const char * const key) const;
	int get_int(const char * const key) const;
	float get_float(const char * const key) const;
	bool get_bool(const char * const key) const;
	
	/* curve specific -- undefined on other values ;-) */
	void finish_curvedata(const float stop, const float start);

	void foreach(hash_callback callback, const void * const user_data);
};

#endif /* !defined(_HASH_H) */
