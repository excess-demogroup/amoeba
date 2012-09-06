/*
 * Simple (and reasonably efficient -- we don't use large hashes so it doesn't
 * really matter that we fix the number of buckets) hash tables, locked to
 * strings.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "main/curve.h"
#include "exception.h"
#include "demolib_prefs.h"

#ifndef __linux__
#define strcasecmp stricmp
#endif

#include "hashtable.h"

#if DEMOLIB_MAINLOOP

unsigned int Hashtable::string_hash(const char *str)
{
        unsigned int hash = 0;

        while (*str) {
                char schar = *str++;
#if CASE_INSENSITIVE_HASHES
                if (schar > 'a' && schar < 'z') schar ^= 32;
#endif

                hash <<= 2;
                hash ^= schar;
                hash += hash >> 30;
        }

        return hash;
}

Hashtable::Hashtable()
{
	for (int i = 0; i < NUM_BUCKETS; i++) {
		this->buckets[i] = NULL;
	}
}

/* all the linked lists will be in reverse order ;-) */
Hashtable::Hashtable(Hashtable *h)
{
	for (int i = 0; i < NUM_BUCKETS; i++) {
		buckets[i] = NULL;
		struct linked_list *ptr =
			(struct linked_list *)h->buckets[i];
	
		while (ptr) {
			struct linked_list *nptr =
				(struct linked_list *)malloc(sizeof(struct linked_list));
			nptr->key = strdup(ptr->key);
			nptr->hash = ptr->hash;
			nptr->obj = ptr->obj;
			nptr->next = buckets[i];
			buckets[i] = nptr;
			ptr = ptr->next;
		}
	}
}

Hashtable::~Hashtable()
{
	for (int i = 0; i < NUM_BUCKETS; i++) {
		struct linked_list *ptr = buckets[i];
		while (ptr) {
			struct linked_list *next = ptr->next;
			free(ptr->key);
			free(ptr);
			ptr = next;
		}
		buckets[i] = NULL;
	}
}

void Hashtable::insert(const char * const key, void * const obj)
{
	const unsigned int hash = string_hash(key);
	const unsigned int partial_hash = hash & BUCKET_MASK;

	this->remove(key);
	
	/* insert in the beginning of the list */
	struct linked_list *ptr =
		(struct linked_list *)malloc(sizeof(struct linked_list));
	ptr->key = strdup(key);
	ptr->hash = hash;
	ptr->obj = obj;
	ptr->next = this->buckets[partial_hash];

	this->buckets[partial_hash] = ptr;
}

void Hashtable::insert(const char * const key, char * const str)
{
	this->insert(key, (void * const)str);
}

/* makes no error if it doesn't exist (make it return a bool?) */
void Hashtable::remove(const char * const key)
{
	const unsigned int hash = string_hash(key);
	const unsigned int partial_hash = hash & BUCKET_MASK;

	struct linked_list *ptr = buckets[partial_hash];
	struct linked_list *prev = NULL;
	while (ptr) {
		if (ptr->hash == hash &&
#if CASE_INSENSITIVE_HASHES
		    strcasecmp(ptr->key, key) == 0
#else
		    strcmp(ptr->key, key) == 0
#endif
		) {
			if (prev == NULL) {
				buckets[partial_hash] = ptr->next;
			} else {
				prev->next = ptr->next;
			}
			free(ptr->key);
			return;
		}
		prev = ptr;
		ptr = ptr->next;
	}
}

void *Hashtable::lookup(const char * const key) const
{
	const unsigned int hash = string_hash(key);
	const unsigned int partial_hash = hash & BUCKET_MASK;
	
	struct linked_list *ptr = buckets[partial_hash];
	while (ptr) {
		if (ptr->hash == hash &&
#if CASE_INSENSITIVE_HASHES
		    strcasecmp(ptr->key, key) == 0
#else
		    strcmp(ptr->key, key) == 0
#endif
		) {
			return ptr->obj;
		}
		ptr = ptr->next;
	}
	return NULL;
}

bool Hashtable::exists(const char * const key) const
{
	return (this->lookup(key) != NULL);
}

char *Hashtable::get_str(const char * const key) const
{
	char *obj = (char *)(this->lookup(key));
	if (obj == NULL)
		 throw new ValueNotSpecifiedException(key);
	return obj;
}

/* might want to check this and get_float() for validity, but okay for now */
int Hashtable::get_int(const char * const key) const
{
	const char * const obj = (const char * const)(this->lookup(key));
	if (obj == NULL)
		 throw new ValueNotSpecifiedException(key);
	return atoi(obj);
}

float Hashtable::get_float(const char * const key) const
{
	const char * const obj = (const char * const)(this->lookup(key));
	if (obj == NULL)
		 throw new ValueNotSpecifiedException(key);
	return atof(obj);
}

bool Hashtable::get_bool(const char * const key) const
{
	const char * const obj = (const char * const)(this->lookup(key));
	if (obj == NULL)
		 throw new ValueNotSpecifiedException(key);
	if (strcmp(obj, "yes") == 0 || strcmp(obj, "true") == 0 ||
	    strcmp(obj, "on") == 0) {
		return true;
	}
	if (strcmp(obj, "no") == 0 || strcmp(obj, "false") == 0 ||
	    strcmp(obj, "off") == 0) {
		return false;
	}
	throw new ValueNotSpecifiedException(key, "Illegal boolean value!");
}

/*
 * free all objects in hash (not keys or the hash itself, use delete for those)
 */
void Hashtable::destroy_values()
{
	for (int i = 0; i < NUM_BUCKETS; i++) {
		struct linked_list *ptr = buckets[i];
		while (ptr) {
			free(ptr->obj);
			ptr->obj = NULL;
			ptr = ptr->next;
		}
	}
}

/* partly obsolete now that we have foreach()? */
void Hashtable::finish_curvedata(const float start, const float stop)
{
	for (int i = 0; i < NUM_BUCKETS; i++) {
		struct linked_list *ptr = buckets[i];
		while (ptr) {
			Curve *c = (Curve *)ptr->obj;
			try {
				c->end_curvepoints(start, stop - start);
			} catch (FatalException *e) {
				/* no such curve */
				char buf[256];
				sprintf(buf, "Curve `%s' missing!", ptr->key);
				throw new FatalException(buf);
			}
			ptr = ptr->next;
		}
	}
}

void Hashtable::foreach(hash_callback callback, const void * const user_data)
{
	for (int i = 0; i < NUM_BUCKETS; i++) {
		struct linked_list *ptr = buckets[i];
		while (ptr) {
			struct linked_list *next = ptr->next;
			(*callback)(ptr->key, ptr->obj, user_data);
			ptr = next;
		}
	}
}
#endif
