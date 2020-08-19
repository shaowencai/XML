#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "xml.h"


struct xml_parser {
	char		*buffer;
	int 		position;
	int 		length;
	
	char		*mallocBuffer;
	int		 	mallocBufferSize;
	int      	mallocIdex;
};

enum xml_parser_offset {
	NO_CHARACTER = -1,
	CURRENT_CHARACTER = 0,
	NEXT_CHARACTER = 1,
};

#define isspace(c)  (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')


static xml_node_t *Xml_New_Item(char *buf,int bufSize,int *pidex)
{
	xml_node_t* node = (xml_node_t*)&buf[*pidex];
	
	*pidex = *pidex + sizeof(xml_node_t) + (8 - (sizeof(xml_node_t)%8));
	
	if( *pidex > bufSize )
	{
		LOG("mallocBufSize is samll!\r\n");
		return 0;
	}
	
	memset(node,0,sizeof(xml_node_t));
	
	return node;
}

static void xml_skip_whitespace(struct xml_parser* parser) 
{
	while (isspace(parser->buffer[parser->position])) 
	{
		if (parser->position + 1 >= parser->length) 
		{
			return;
		} 
		else 
		{
			parser->position++;
		}
	}
}


static char xml_parser_peek(struct xml_parser* parser, int n) 
{
	int position = parser->position;

	while (position < parser->length) 
	{
		if (!isspace(parser->buffer[position])) 
		{
			if (n == 0) 
			{
				return parser->buffer[position];
			} 
			else 
			{
				--n;
			}
		}

		position++;
	}

	return 0;
}


static void xml_parser_consume(struct xml_parser* parser,int n) 
{
	parser->position += n;

	if (parser->position >= parser->length) 
	{
		parser->position = parser->length - 1;
	}
	return;
}


static char* xml_parse_tag_end(struct xml_parser* parser) 
{
	int start = parser->position;
	int length = 0;

	while (start + length < parser->length) 
	{
		uint8_t current = xml_parser_peek(parser, CURRENT_CHARACTER);

		if ('>' == current) 
		{
			xml_skip_whitespace(parser);
			break;
		}
		else 
		{
			xml_parser_consume(parser, 1);
			length++;
		}
	}

	if ('>' != xml_parser_peek(parser, CURRENT_CHARACTER)) 
	{
		return 0;
	}

	xml_parser_consume(parser, 1);

	parser->buffer[start+length] = '\0';

	return &parser->buffer[start];
}

static char* xml_parse_tag_open(struct xml_parser* parser) 
{
	xml_skip_whitespace(parser);

	if ('<' != xml_parser_peek(parser, CURRENT_CHARACTER)) 
	{
		return 0;
	}

	xml_parser_consume(parser, 1);

	return xml_parse_tag_end(parser);
}

static char* xml_strtok_r(char *str, const char *delim, char **nextp) 
{
	char *ret;

	if (str == NULL) 
	{
		str = *nextp;
	}

	str += strspn(str, delim);

	if (*str == '\0') 
	{
		return NULL;
	}

	ret = str;

	str += strcspn(str, delim);

	if (*str) 
	{
		*str++ = '\0';
	}

	*nextp = str;

	return ret;
}


static void xml_find_attributes(char* tag_open,struct xml_node* node) 
{
	char* rest = NULL;
	char* token;
	char  str_name[256];
	char  str_content[256];
	char* start_name;
	char* start_content;
	int   strLen = 0;

	token = xml_strtok_r(tag_open, " ", &rest); // skip the first value
	
	if(token == NULL) 
	{
		return;
	}

	for(token=rest; *token;token=rest) 
	{
		// %s=\"%s\" wasn't working for some reason, ugly hack to make it work
		memset(str_name,0,sizeof(str_name));
		memset(str_content,0,sizeof(str_content));
		
		if(sscanf(token, "%[^=\" ]=\"%[^\"]", str_name, str_content) != 2 &&
		   sscanf(token, "%[^=\" ] = \"%[^\"]", str_name, str_content) != 2 &&
		   sscanf(token, "%[^=\' ]=\'%[^\']", str_name, str_content) != 2 &&
		   sscanf(token, "%[^=\' ] = \'%[^\']", str_name, str_content) != 2	&&
		   sscanf(token, "%[^=\' ]= \'%[^\']", str_name, str_content) != 2	&&
		   sscanf(token, "%[^=\' ] =\'%[^\']", str_name, str_content) != 2	&&
		   sscanf(token, "%[^=\" ]= \"%[^\"]", str_name, str_content) != 2 &&
		   sscanf(token, "%[^=\" ] =\"%[^\"]", str_name, str_content) != 2) 
		{
			rest ++;
			continue;
		}
		
		LOG("%s:%s\r\n",str_name,str_content);
		
		start_name = token;
		
		strLen = strlen(str_name);
		
		while(memcmp(start_name,str_name,strLen) != 0) start_name++;
		
		start_name[strlen(str_name)] = '\0';
		
		start_content = &start_name[strLen + 2];
		
		strLen = strlen(str_content);
		
		while(memcmp(start_content,str_content,strLen) != 0) start_content++;
		
		start_content[strlen(str_content)] = '\0';

		node->attributes[node->attrNum].name = start_name;
		node->attributes[node->attrNum].value = start_content;
		node->attrNum++;
		
		rest = start_content + strlen(str_content) + 1;
		
		while(*rest == ' ' || *rest == '\t')rest++;
	}
	return;
}


static char* xml_parse_content(struct xml_parser* parser) 
{
	int start = parser->position;
	int length = 0;
	int length1 = 0;

	xml_skip_whitespace(parser);
	
	/* Consume until `<' is reached
	 */
	while (start + length < parser->length) 
	{
		uint8_t current = xml_parser_peek(parser, CURRENT_CHARACTER);

		if ('<' == current) 
		{
			break;
		} 
		else 
		{
			xml_parser_consume(parser, 1);
			length++;
		}
	}

	/* Next character must be an `<' or we have reached end of file
	 */
	if ('<' != xml_parser_peek(parser, CURRENT_CHARACTER)) 
	{
		return 0;
	}
	
	length1 = length;

	/* Ignore tailing whitespace
	 */
	while ((length > 0) && isspace(parser->buffer[start + length - 1])) 
	{
		length--;
	}
	
	if(length1 != length)
	{
		parser->buffer[start + length] = '\0';
	}
	
	return &parser->buffer[start];
}


static char* xml_parse_tag_close(struct xml_parser* parser) 
{
	xml_skip_whitespace(parser);

	/* Consume `</'
	 */
	if (  ('<' != xml_parser_peek(parser, CURRENT_CHARACTER))
		||('/' != xml_parser_peek(parser, NEXT_CHARACTER)) ) 
	{
		return 0;
	}
	
	parser->buffer[parser->position] = '\0';
	
	xml_parser_consume(parser, 2);

	return xml_parse_tag_end(parser);
}


static xml_node_t* xml_parse_node(struct xml_parser* parser) 
{
	char* tag_open = 0;
	char* tag_close = 0;
	char* content = 0;
	xml_node_t* next = 0;

	xml_node_t* node = Xml_New_Item(parser->mallocBuffer,parser->mallocBufferSize,&(parser->mallocIdex));
	
	if(!node)
	{
		return 0;
	}
	
	while(1)
	{
		tag_open = xml_parse_tag_open(parser);
	
		if (!tag_open) 
		{
			return 0;
		}
		
		if(tag_open[0] == '?' && tag_open[strlen(tag_open)-1] == '?')
		{
			continue;
		}
		
		if(tag_open[0] == '!' && tag_open[1] == '-' && tag_open[2] == '-' && tag_open[strlen(tag_open)-1] == '-' && tag_open[strlen(tag_open)-2] == '-')
		{
			continue;
		}
		
		/* If tag ends with `/' it's self closing, skip content lookup */
		if (strlen(tag_open) > 0 && '/' == tag_open[strlen(tag_open) - 1]) 
		{
			node->name = tag_open;
			xml_find_attributes(tag_open,node);
				
			//如果是<tag/>的形式 需要将最后的/消除掉 
			if('/' == tag_open[strlen(tag_open) - 1]) 
			{
				tag_open[strlen(tag_open) - 1] = '\0';
			}
			
			return node;
		}
	
		break;
	}
	
	node->name = tag_open;

	xml_find_attributes(tag_open,node);

	/* If the content does not start with '<', a text content is assumed
	 */
	if ('<' != xml_parser_peek(parser, CURRENT_CHARACTER)) 
	{
		content = xml_parse_content(parser);

		if (!content) 
		{
			LOG("xml_parse_content fail\r\n");
			return 0;
		}
		node->content = content;
	}
	else 
	{
		while ('/' != xml_parser_peek(parser, NEXT_CHARACTER)) 
		{
			struct xml_node* child = xml_parse_node(parser);
			
			if (!child) 
			{
				LOG("xml_parse_node fail\r\n");
				return 0;
			}
			
			if(! node->child)
			{
				node->child = child;
			}
			else
			{
				next = node->child;
				
				while(next->next)
				{
					next = next->next;
				}
				next->next = child;
			}
		}
	}

	tag_close = xml_parse_tag_close(parser);
	
	if (!tag_close) 
	{
		return 0;
	}

	if (strcmp(tag_open, tag_close)) 
	{
		LOG("tag_open(%s) != tag_close(%s)\r\n",tag_open,tag_close);
		return 0;
	}

	return node;
}


xml_node_t* xml_parse_document(char* buffer, int length,char *mallocBuf,int mallocBufSize) 
{
	struct xml_parser parser = {
		.buffer = buffer,
		.position = 0,
		.length = length,
		.mallocBuffer = mallocBuf,
		.mallocBufferSize = mallocBufSize,
		.mallocIdex = 0,
	};
	xml_node_t* root;

	if (!length || !mallocBufSize) 
	{
		LOG("params is error"); 
		return 0;
	}

	root = xml_parse_node(&parser);

	if (!root) 
	{
		return 0;
	}

	
	return root;
}


/* 供外部调用的接口，获取当前节点下孩子的数目 */
int xml_node_child_num(xml_node_t *node)							
{
	xml_node_t *c= node ? node->child:0;
	
	int i=0;
	
	while(c)i++,c=c->next;
	
	return i;
}

/* 供外部调用的接口，通过当前组下标得到指定元素  */
xml_node_t *xml_get_node_nth_child(xml_node_t *node,int nth)				
{
	if(!node)return 0;
	
	xml_node_t *c=node->child;  
	
	while (c && nth>0) nth--,c=c->next;
	
	return c;
}


char* xml_node_name(xml_node_t* node)
{
	return node ? node->name : (char *)"";
}

char* xml_node_content(xml_node_t* node)
{
	return node ? node->content :  (char *)"";
}


int xml_node_attr_num(xml_node_t *node)
{
	return node ? node->attrNum : 0;
}

char *xml_get_node_nth_attr_key(xml_node_t *node,int nth)				
{
	if(!node || nth >= node->attrNum)return 0;
	
	return node->attributes[nth].name;
}

char *xml_get_node_nth_attr_value(xml_node_t *node,int nth)				
{
	if(!node || nth >= node->attrNum)return 0;
	
	return node->attributes[nth].value;
}

char *xml_get_node_value_by_key(xml_node_t *node,char *key)				
{
	int i = 0;
	
	if(!node || !key)return 0;
	
	for(i=0; i<node->attrNum; i++)
	{
		if(0 == strcmp(node->attributes[i].name,key))
		{
			break;
		}
	}
	
	if(i == node->attrNum)
	{
		return  (char *)"";
	}
	
	return node->attributes[i].value;
}


xml_node_t *xml_get_first_node_by_depth(xml_node_t *node,int depth)				
{
	if(depth < 0) return 0;
	
	while(depth && node)
	{
		node = node->child;
		depth --;	
	}
	
	return node;
}

