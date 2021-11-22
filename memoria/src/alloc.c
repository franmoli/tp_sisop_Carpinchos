#include "alloc.h"

// Memfree -> Libero alloc (flag isFree en true), me fijo el anterior y posterior y los unifico
// TODO -> Meter paginacion (Mati gatooo)
void freeAlloc(t_paquete *paquete)
{

    uint32_t inicio = tamanio_memoria;

    uint32_t direccion = deserializar_alloc(paquete);
    uint32_t carpincho_id = 2;
    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(carpincho_id);

    if (!direccionValida(direccion, carpincho_id))
    {
        //MATE FREE FAIÑT
    }
    //Traigo de memoria el alloc
    t_heap_metadata *alloc = traerAllocDeMemoria(direccion);

    alloc->isFree = true;
    uint32_t next = alloc->nextAlloc;
    uint32_t back = alloc->prevAlloc;

    int paginaAlloc = getPaginaByDireccion(back);
    t_pagina *pagina_alloc = list_get(tabla_paginas->paginas, paginaAlloc);
    pagina_alloc->tamanio_ocupado -= next - back - sizeof(t_heap_metadata);
    pagina_alloc->cantidad_contenidos -= 1;

    t_heap_metadata *anterior;
    t_heap_metadata *posterior;
    bool hayAnterior = false;
    bool hayPosterior = false;

    if (alloc->prevAlloc != 0)
    { // SI EXISTE ANTERIOR TRAERLO
        anterior = traerAllocDeMemoria(alloc->prevAlloc);
        if (anterior->isFree)
            hayAnterior = true;
    }
    if (next != NULL)
    { //SI EXISTE PROXIMO
        posterior = traerAllocDeMemoria(next);
        if (posterior->isFree)
            hayPosterior = true;
    }
    if (hayAnterior && hayPosterior)
    {
        anterior->nextAlloc = posterior->nextAlloc;

        int paginaNextAlloc = getPaginaByDireccion(next);
        t_pagina *pagina_allocNext = list_get(tabla_paginas->paginas, paginaNextAlloc);
        pagina_allocNext->tamanio_ocupado -= posterior->nextAlloc - next - sizeof(t_heap_metadata);
        pagina_allocNext->cantidad_contenidos -= 1;

        if (list_size(tabla_paginas->paginas) == pagina_allocNext->numero_pagina && pagina_allocNext->cantidad_contenidos == 1)
        {
            list_remove(tabla_paginas->paginas, pagina_allocNext);
            free(pagina_allocNext);
            tabla_paginas->paginas_en_memoria -= 1;
        }

        free(alloc);
        free(posterior);
        guardarAlloc(anterior, back);
        return;
    }
    if (hayAnterior)
    {
        anterior->nextAlloc = alloc->nextAlloc;
        int paginaAlloc = getPaginaByDireccion(back);
        t_pagina *pagina_alloc = list_get(tabla_paginas->paginas, paginaAlloc);
        pagina_alloc->tamanio_ocupado -= next - back - sizeof(t_heap_metadata);
        pagina_alloc->cantidad_contenidos -= 1;
        free(alloc);
        guardarAlloc(anterior, back);
        return;
    }

    if (hayPosterior)
    {
        alloc->nextAlloc = posterior->nextAlloc;
        int paginaNext = getPaginaByDireccion(next);
        t_pagina *pagina_Next = list_get(tabla_paginas->paginas, paginaNext);
        if (list_size(tabla_paginas->paginas) == pagina_Next->numero_pagina && pagina_Next->cantidad_contenidos == 1)
        {
            list_remove(tabla_paginas->paginas, paginaNext);
            free(pagina_Next);
            tabla_paginas->paginas_en_memoria -= 1;
        }
        else
        {
            pagina_Next->cantidad_contenidos -= 1;
            //restar tamanio ocupado
            //pagina del alloc actual restar tamanio
        }
        free(posterior);
        guardarAlloc(alloc, direccion);
        return;
    }
}

bool direccionValida(uint32_t direccion, uint32_t carpincho_id)
{

    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(carpincho_id);

    bool esValida = true;
    int numero_pagina = getPaginaByDireccion(direccion);
    log_info(logger_memoria, "Pagina:%d", numero_pagina);
    t_pagina *pagina = list_get(tabla_paginas->paginas, numero_pagina);
    if (!pagina->bit_presencia)
        esValida = false;

    return esValida;
}
t_heap_metadata *traerAllocDeMemoria(uint32_t direccion)
{

    t_heap_metadata *data = malloc(sizeof(t_heap_metadata));

    uint32_t offset = 0;
    memcpy(&data->prevAlloc, direccion + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&data->nextAlloc, direccion + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&data->isFree, direccion + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    return data;
}
void guardarAlloc(t_heap_metadata *data, uint32_t direccion)
{

    uint32_t offset = 0;
    memcpy(direccion + offset, &data->prevAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(direccion + offset, &data->nextAlloc, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(direccion + offset, &data->isFree, sizeof(uint8_t));
}

void memAlloc(t_paquete *paquete)
{

    t_malloc_serializado *mallocDeserializado = deserializar_alloc(paquete);

    int size = mallocDeserializado->size_reservar;
    int carpincho_id = mallocDeserializado->carpincho_id;

    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(carpincho_id);
    uint32_t inicio = tamanio_memoria;

    if (list_size(tabla_paginas->paginas) == 0)
    {
        int numero_pagina = solicitarPaginaNueva(carpincho_id);
        t_pagina *pagina = list_get(tabla_paginas->paginas, numero_pagina);
        crearPrimerHeader(pagina);
        t_heap_metadata *header = traerAllocDeMemoria(inicio);
        agregarPagina(pagina, header, header->nextAlloc, size,false);
    }
    else
    {
        int paginaDisponible = getPrimeraPaginaDisponible(size, tabla_paginas);
        t_heap_metadata *data = traerAllocDeMemoria(inicio);
        uint32_t nextAnterior = tamanio_memoria;
        while (data->nextAlloc != NULL)
        {
            if (data->isFree)
            {

                //Estoy en un alloc libre y no es el ultimo, hacer si entra totalmente, sino que siga

                uint32_t sizeAlloc;
                if (data->prevAlloc == NULL)
                {
                    log_info(logger_memoria, "Primer Alloc Posiblemente Libre");
                    sizeAlloc = data->nextAlloc - inicio - sizeof(t_heap_metadata);
                }
                else
                {
                    log_info(logger_memoria, "Alloc Del Medio Posiblemente Libre");
                    sizeAlloc = data->nextAlloc - nextAnterior - sizeof(t_heap_metadata);
                }

                if (size == sizeAlloc)
                {

                    log_info(logger_memoria, "Hay un Alloc Libre disponible paraa usar del mismo tamanio requerido");
                    //Uso este alloc para guardar

                    int pagina = getPaginaByDireccion(nextAnterior);
                    t_pagina *paginaAlloc = list_get(tabla_paginas->paginas, pagina);
                    if (paginaAlloc->bit_presencia)
                    {
                    }
                    paginaAlloc->cantidad_contenidos += 1;
                    paginaAlloc->tamanio_ocupado += size;

                    data->isFree = false;
                    guardarAlloc(data, nextAnterior);
                    return;
                }
                else if (sizeAlloc > size + sizeof(t_heap_metadata))
                {

                    log_info(logger_memoria, "Encontre un Alloc libre que tiene un tamanio mayor la requerido. Hay que separarlo");

                    data->isFree = false;
                    data->nextAlloc = nextAnterior + sizeof(t_heap_metadata) + size;

                    guardarAlloc(data, nextAnterior);

                    data->isFree = true;
                    data->prevAlloc = nextAnterior;
                    data->nextAlloc = nextAnterior + sizeof(t_heap_metadata) * 2 + sizeAlloc;

                    guardarAlloc(data, nextAnterior + sizeof(t_heap_metadata) + size);
                }
            }
            nextAnterior = data->nextAlloc;
            data = traerAllocDeMemoria(data->nextAlloc);
        }
        //Nuevo Alloc
        int paginaLastAlloc = getPaginaByDireccion(nextAnterior);
        t_pagina *pagina = list_get(tabla_paginas->paginas, paginaLastAlloc);
        agregarPagina(pagina, data, nextAnterior, size,false);
    }
}
void crearPrimerHeader(t_pagina *pagina)
{
    uint32_t inicio = tamanio_memoria;
    t_heap_metadata *data = malloc(sizeof(t_heap_metadata));
    data->nextAlloc = inicio + sizeof(t_heap_metadata);
    data->prevAlloc = NULL;
    data->isFree = false;
    guardarAlloc(data, inicio);
    pagina->cantidad_contenidos += 1;
    pagina->tamanio_ocupado += sizeof(t_heap_metadata);
    free(data);
}

void agregarPagina(t_pagina *pagina, t_heap_metadata *data, uint32_t nextAnterior, uint32_t size, bool ultimo)
{

    uint32_t inicio = tamanio_memoria;
    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(pagina->carpincho_id);

    if (pagina->tamanio_ocupado < config_memoria->TAMANIO_PAGINA)
    {
        if (pagina->tamanio_ocupado + size <= config_memoria->TAMANIO_PAGINA)
        {
            //entra completo
            if (ultimo)
            {
                data->prevAlloc = nextAnterior;
                nextAnterior = data->nextAlloc;
                data->nextAlloc = NULL;
                data->isFree = true;

                guardarAlloc(data, nextAnterior);
                pagina->cantidad_contenidos += 1;
                pagina->tamanio_ocupado += size;
                return;
            }
            else
            {
                if (data->prevAlloc == NULL)
                {
                    data->nextAlloc = nextAnterior + size;
                }
                else
                {
                    data->nextAlloc = nextAnterior + size + sizeof(t_heap_metadata);
                }
                data->isFree = false;
                if (data->prevAlloc == NULL)
                    nextAnterior = inicio;
            }

            guardarAlloc(data, nextAnterior);
            pagina->cantidad_contenidos += 1;
            pagina->tamanio_ocupado += size;

            agregarPagina(pagina, data, nextAnterior, sizeof(t_heap_metadata), true);
        }
        else
        {
            //ocupo el restante y pido otra
            int restante = size - (config_memoria->TAMANIO_PAGINA - pagina->tamanio_ocupado);
            if (list_size(tabla_paginas->paginas) + 1 <= tabla_paginas->paginas_totales_maximas)
            {

                data->nextAlloc = nextAnterior + size + sizeof(t_heap_metadata);
                data->isFree = false;
                guardarAlloc(data, nextAnterior);
                pagina->cantidad_contenidos += 1;
                pagina->tamanio_ocupado += (size - restante);

                int numero_pagina = solicitarPaginaNueva(pagina->carpincho_id);
                pagina = list_get(tabla_paginas->paginas,numero_pagina);

                agregarPagina(pagina, data, nextAnterior, restante + sizeof(t_heap_metadata), true);
            }
        }
    }
    else
    {
        int numero_pagina = solicitarPaginaNueva(pagina->carpincho_id);
        pagina = list_get(tabla_paginas->paginas, numero_pagina);
        data->isFree = true;
        data->prevAlloc = nextAnterior;

        nextAnterior = data->nextAlloc;

        data->nextAlloc = NULL;
        guardarAlloc(data, nextAnterior);
        pagina->cantidad_contenidos += 1;
        pagina->tamanio_ocupado += size;
    }
}
t_heap_metadata *getLastHeapFromPagina(int pagina, int carpincho_id)
{

    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(carpincho_id);
    t_pagina *paginaBuscada = list_get(tabla_paginas->paginas, pagina);
    t_contenidos_pagina *contenidoUltimo = getLastContenidoByPagina(paginaBuscada);
    t_heap_metadata *metadata = traerAllocDeMemoria(contenidoUltimo->dir_comienzo);
    return metadata;
}

void mostrarAllocs(int carpincho_id)
{
    t_tabla_paginas *tabla_paginas = buscarTablaPorPID(carpincho_id);
    t_pagina *paginaLeida = list_get(tabla_paginas->paginas, 0);
    if (paginaLeida == NULL)
        return;

    uint32_t inicio = tamanio_memoria + config_memoria->TAMANIO_PAGINA * paginaLeida->numero_pagina;
    t_heap_metadata *data = traerAllocDeMemoria(inicio);
    while (data->nextAlloc != NULL)
    {
        printf("Prev Alloc: %d  Next Alloc: %d.IsFree:%d \n", data->prevAlloc, data->nextAlloc, data->isFree);
        data = traerAllocDeMemoria(data->nextAlloc);
    }
}