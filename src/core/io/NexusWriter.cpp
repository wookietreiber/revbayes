#include "AbstractTaxonData.h"
#include "CharacterState.h"
#include "NexusWriter.h"

using namespace RevBayesCore;


/**
 * Default constructor.
 * The default constructor does nothing except allocating the object.
 *
 * \param[in]   fileName    The name of the file into which the objects is to be written.
 */
NexusWriter::NexusWriter( const std::string &fn ) :
    fileName( fn )
{
    
}


/**
 * Close the file stream.
 */
void NexusWriter::closeStream( void ) 
{
    
    // close the stream
    outStream.close();
}


/**
 * Open the file stream to a file with the name used in the constructor.
 */
void NexusWriter::openStream( void ) 
{
    
    // open the stream to the file
    outStream.open( fileName.c_str(), std::fstream::out );
    
    // write header line
    outStream << "#NEXUS" << std::endl;
    
}


/**
 * This method simply writes a character data object into a file in Nexus format.
 *
 * \param[in]   data        The character data object which is written out.
 */
void NexusWriter::writeNexusBlock(const AbstractCharacterData &data) 
{
    
    // write initial lines of the character block
    outStream << std::endl;
    outStream << "Begin data;" << std::endl;
    outStream << "Dimensions ntax=" << data.getNumberOfTaxa() << " nchar=" << data.getNumberOfCharacters() << ";" << std::endl;
    outStream << "Format datatype=" << data.getDatatype() << " ";
//    if ( data.getDatatype() == "Standard" )
//    {
//        outStream << "symbols=\"" << data.getTaxonData(0).getCharacter(0).getStateLabels() << "\" ";
//    }
    outStream << "missing=? gap=-;" << std::endl;
    outStream << "Matrix" << std::endl;

    
    const std::vector<std::string> &taxonNames = data.getTaxonNames();
    for (std::vector<std::string>::const_iterator it = taxonNames.begin();  it != taxonNames.end(); ++it) 
    {
        outStream << *it << "   " << std::endl;
        const AbstractTaxonData &taxon = data.getTaxonData( *it );
        size_t nChars = taxon.getNumberOfCharacters();
        for (size_t i = 0; i < nChars; ++i) 
        {
            if ( !data.isCharacterExcluded( i ) )
            {
                const CharacterState &c = taxon.getCharacter( i );  
                outStream << c.getStringValue();
            }
        }
        
        outStream << std::endl;
    }
    
    outStream << ";" << std::endl;
    outStream << "End;" << std::endl;
    outStream << std::endl;
    
}