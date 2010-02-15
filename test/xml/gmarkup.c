#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

gchar *current_animal_noise = NULL;

/* The handler functions. */

void start_element (GMarkupParseContext *context,
    const gchar         *element_name,
    const gchar        **attribute_names,
    const gchar        **attribute_values,
    gpointer             user_data,
    GError             **error) {

  const gchar **name_cursor = attribute_names;
  const gchar **value_cursor = attribute_values;

  while (*name_cursor) {
    if (strcmp (*name_cursor, "noise") == 0)
      current_animal_noise = g_strdup (*value_cursor);

    name_cursor++;
    value_cursor++;
  }
}

void text(GMarkupParseContext *context,
    const gchar         *text,
    gsize                text_len,
    gpointer             user_data,
    GError             **error)
{
  /* Note that "text" is not a regular C string: it is
   * not null-terminated. This is the reason for the
   * unusual %*s format below.
   */
  if (current_animal_noise)
    printf("I am a %*s and I go %s. Can you do it?\n",
        text_len, text, current_animal_noise);
}

void end_element (GMarkupParseContext *context,
    const gchar         *element_name,
    gpointer             user_data,
    GError             **error)
{
  if (current_animal_noise)
    {
      g_free (current_animal_noise);
      current_animal_noise = NULL;
    }
}

/* The list of what handler does what. */
static GMarkupParser parser = {
  start_element,
  end_element,
  text,
  NULL,
  NULL
};

/* Code to grab the file into memory and parse it. */
int main() {
  char *text;
  gsize length;
  GMarkupParseContext *context = g_markup_parse_context_new (
      &parser,
      0,
      NULL,
      NULL);

  /* seriously crummy error checking */

  if (g_file_get_contents ("simple.xml", &text, &length, NULL) == FALSE) {
    printf("Couldn't load XML\n");
    exit(255);
  }

  if (g_markup_parse_context_parse (context, text, length, NULL) == FALSE) {
    printf("Parse failed\n");
    exit(255);
  }

  g_free(text);
  g_markup_parse_context_free (context);
}
/* EOF */
